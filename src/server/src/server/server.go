package server

/*
#include "stdafx.h"
#include "stdlib.h"
#include "dlf.h"

#cgo LDFLAGS: -ldl
*/
import "C"
import (
	"net"
	"strconv"
	"unsafe"
	"strings"
	"encoding/binary"
	"io"
	//"log"
	//"net/http"
	//_ "net/http/pprof"
	"sync"
)

var zero struct{}

type Server struct {
	l net.Listener
}

type Client struct {
	c          net.Conn
	cInfo      C.connect_info
	dst        net.Conn
	rw         BssRW
	chans2r    chan []byte
	chans2c    chan []byte
	chanStart  chan struct{}
	chans2rEnd chan struct{}
	chanr2sEnd chan struct{}
	chanc2sEnd chan struct{}
	chans2cEnd chan struct{}
	wg         *sync.WaitGroup
}

func (s *Server) Start() {
	err := initConfig()
	//go func() {
	//	log.Println(http.ListenAndServe("localhost:6060", nil))
	//}()
	if err != nil {
		PPrintf(err)
	}
	loadSo(DylibPath)
	s.l, err = net.Listen("tcp", ":"+strconv.Itoa(Port))
	if err != nil {
		PPrintf(err)
	}
	for {
		c, err := s.l.Accept()
		if err != nil {
			EPrintf("%v", err)
		}
		go s.handle(c)
	}
	closeSo()
}

func (s *Server) handle(c net.Conn) {
	var cli Client
	if c == nil {
		return
	}
	cli.c = c
	var pass C.binary_safe_string
	pass.length = C.uint32_t(len(Passphrase))
	pass.str = C.CString(Passphrase)
	cli.cInfo.passphrase = pass
	defer C.free(unsafe.Pointer(cli.cInfo.passphrase.str))
	if ok := s.auth(&cli); ok {
		s.proxy(&cli)
	}
	return
}

func (s *Server) auth(c *Client) bool {
	rAddr := c.c.RemoteAddr().(*net.TCPAddr)
	ip := rAddr.IP.To16()
	if ip == nil {
		EPrintf("[Auth]IP Convert Error!")
		return false
	}
	p := uint16(rAddr.Port)
	for i, data := range ip {
		c.cInfo.server_ip[i] = C.uint8_t(data)
	}
	c.cInfo.server_port = C.uint16_t(p)
	c.cInfo.connect_fd = -1
	//Because of Go net package, We can't use connect_fd for anything else.
	rw := BssRW{c.c}
	packet := new(C.binary_safe_string)
	err := rw.ReadB(packet)
	if err != nil {
		return false
	}
	isClose := C.bool(false)
	reply := C.call_server_make_auth_reply(&c.cInfo, *packet, &isClose)
	rw.WriteB(reply)
	if bool(isClose) {
		DPrintf("[Auth]Auth Error.")
		return false
	} else {
		DPrintf("[Auth]Auth Success.")
		return true
	}
}

func (s *Server) proxy(c *Client) {
	c.chans2r = make(chan []byte, 1)
	c.chans2c = make(chan []byte, 1)
	c.chanStart = make(chan struct{})
	c.chanc2sEnd = make(chan struct{}, 2)
	c.chanr2sEnd = make(chan struct{}, 2)
	c.chans2rEnd = make(chan struct{}, 2)
	c.chans2cEnd = make(chan struct{}, 2)
	c.wg = new(sync.WaitGroup)
	c.rw = BssRW{c.c}
	defer C.call_server_connection_close(&c.cInfo)
	defer DPrintf("[Proxy]Connection %v Closed.", c.cInfo)
	defer c.wg.Wait()
	go s.deals2r(c)
	go s.dealr2s(c)
	go s.deals2c(c)
	c.wg.Add(3)
	if err := s.checkFirst(c); err != nil {
		EPrintf("%v", err)
		c.chans2rEnd <- zero
		return
	}
	go s.dealc2s(c)
	c.wg.Add(1)
	return
}

func (s *Server) checkFirst(c *Client) error {
	packet := new(C.binary_safe_string)
	err := c.rw.ReadB(packet)
	if err != nil {
		return err
	}
	datac2s := C.call_server_decode(&c.cInfo, *packet)
	defer C.free(unsafe.Pointer(datac2s.payload.str))
	datac2sStr := C.GoStringN(datac2s.payload.str, C.int(datac2s.payload.length))
	DPrintf("[First]Read bytes: %v", []byte(C.GoStringN(datac2s.payload.str, C.int(datac2s.payload.length))))
	if strings.HasPrefix(datac2sStr, "__m_str_dns_head_32__snakesocks_") {
		datas2cStr := make([]byte, 0)
		datas2cStr = append(datas2cStr, []byte("__m_str_dns_head_32__snakesocks_")...)
		var answer C.client_query
		datac2sStr = datac2sStr[32:]
		ip, err := net.LookupIP(datac2sStr)
		if err != nil {
			datas2cStr = append(datas2cStr, strconv.Itoa(-1)...)
			answer.payload.length = C.uint32_t(len(datas2cStr))
			answer.payload.str = C.CString(string(datas2cStr))
			datas2c := C.call_server_encode(&c.cInfo, answer)
			errn := c.rw.WriteB(datas2c)
			if errn != nil {
				EPrintf("[Dns]Write Error: %v", err)
				return errn
			}
			return err
		} else {
			DPrintf("[Dns]IP Converted: %v", ip)
			for i, data := range ip[0].To16() {
				answer.destination_ip[i] = C.uint8_t(data)
				c.cInfo.server_ip[i] = C.uint8_t(data)
			}
			datas2cStr = append(datas2cStr, byte(6))
			datas2cStr = append(datas2cStr, ip[0].To16()...)
			answer.payload.length = C.uint32_t(len(datas2cStr))
			answer.payload.str = C.CString(string(datas2cStr))
			datas2c := C.call_server_encode(&c.cInfo, answer)
			err = c.rw.WriteB(datas2c)
			if err != nil {
				EPrintf("[Dns]Write Error: %v", err)
				return err
			}
		}
	} else {
		if c.dst == nil {
			if err := s.dial(c, datac2s); err != nil {
				return err
			}
			c.chanStart <- zero
			c.chanStart <- zero
		}
		c.chans2r <- []byte(datac2sStr)
	}
	return nil
}

func (s *Server) dial(c *Client, datac2s C.client_query) error {
	ip := C.GoBytes(unsafe.Pointer(&datac2s.destination_ip), 16)
	portB := make([]byte, 2)
	binary.LittleEndian.PutUint16(portB, uint16(datac2s.destination_port))
	port := binary.BigEndian.Uint16(portB)
	DPrintf("[Connect]Connected to %v:%v", net.IP(ip).To16().String(), port)
	dst, err := net.Dial("tcp", net.JoinHostPort(net.IP(ip).String(), strconv.Itoa(int(port))))
	if err != nil {
		return err
	}
	c.dst = dst
	return nil
}

func (s *Server) dealc2s(c *Client) {
	defer func() {
		c.chans2rEnd <- zero
		c.wg.Done()
		DPrintf("[c2s]Closed")
	}()
	packetc2s := new(C.binary_safe_string)
	var datac2s C.client_query
	for {
		err := c.rw.ReadB(packetc2s)
		if err != nil {
			if err != io.EOF {
				EPrintf("%v", err)
			}
			return
		}
		datac2s = C.call_server_decode(&c.cInfo, *packetc2s)
		data := []byte(C.GoStringN(datac2s.payload.str, C.int(datac2s.payload.length)))
		DPrintf("[Read]Read From client: %v", data)
		if c.dst == nil {
			if err := s.dial(c, datac2s); err != nil {
				return
			}
			c.chanStart <- zero
			c.chanStart <- zero
		}

		C.free(unsafe.Pointer(datac2s.payload.str))
		c.chans2r <- data
	}
}

func (s *Server) deals2r(c *Client) {
	defer func() {
		c.chanr2sEnd <- zero
		c.wg.Done()
		DPrintf("[s2r]Closed")
	}()
	select {
	case <-c.chans2rEnd:
		return
	case <-c.chanStart:
	}
	for {
		select {
		case datas2r := <-c.chans2r:
			_, err := c.dst.Write(datas2r)
			if err != nil {
				if err != io.EOF {
					EPrintf("%v", err)
				}
				return
			}
			DPrintf("[Deal]Write to remote server: %v", datas2r)
		case <-c.chans2rEnd:
			return
		}

	}
}

func (s *Server) dealr2s(c *Client) {
	defer func() {
		c.chans2cEnd <- zero
		c.wg.Done()
		DPrintf("[r2s]Closed")
	}()
	select {
	case <-c.chanr2sEnd:
		return
	case <-c.chanStart:
	}
	eCh := make(chan error, 1)
	go func(eCh chan error) {
		datar2s := make([]byte, 1024)
		for {
			n, err := c.dst.Read(datar2s)
			if err != nil {
				if err != io.EOF {
					EPrintf("%v", err)
				}
				eCh <- err
				return
			}
			DPrintf("[Deal]Read from remote server: %v", datar2s)
			data := make([]byte, n)
			copy(data, datar2s[:n])
			c.chans2c <- data
		}
	}(eCh)
	select {
	case <-eCh:
		return
	case <-c.chanr2sEnd:
		return
	}
}

func (s *Server) deals2c(c *Client) {
	defer func() {
		c.chans2cEnd <- zero
		c.wg.Done()
		DPrintf("[s2c]Closed")
	}()

	var datas2c C.client_query
	for {
		select {
		case data := <-c.chans2c:
			datas2c.payload.length = C.uint32_t(len(data))
			datas2c.payload.str = C.CString(string(data))
			packets2c := C.call_server_encode(&c.cInfo, datas2c)
			err := c.rw.WriteB(packets2c)
			if err != nil {
				if err != io.EOF {
					EPrintf("%v", err)
				}
				return
			}
			DPrintf("[Write]Write to client: %v", data)
		case <-c.chans2cEnd:
			return
		}
	}
}
