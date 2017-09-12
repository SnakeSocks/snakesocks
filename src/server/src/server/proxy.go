package server

/*
#include "stdafx.h"
#include "stdlib.h"
#include "dlf.h"

#cgo LDFLAGS: -ldl
*/
import "C"

import (
	"encoding/binary"
	"errors"
	"net"
	"strconv"
	"unsafe"
	"io"
)

func proxyGo(client net.Conn, connectInfo C.connect_info) {
	dstConn, err := getAddress(client, connectInfo)
	if err != nil {
		EPrintf("%v", err)
		return
	}
	chanProxyEnd := make(chan bool)
	go read(client, connectInfo, dstConn, chanProxyEnd)
	go write(client, connectInfo, dstConn, chanProxyEnd)
	sig := <- chanProxyEnd
	if sig {
		dstConn.Close()
		_ = <-chanProxyEnd
		close(chanProxyEnd)
		C.call_server_connection_close(&connectInfo)
		return
	}
}

func getAddress(client net.Conn, connectInfo C.connect_info) (net.Conn, error) {
	var data C.binary_safe_string
	ok := readBinarySafeString(client, &data)
	if !ok {
		return nil, errors.New("get address error!")
	}
	query := C.call_server_decode(&connectInfo, data)
	defer C.free(unsafe.Pointer(query.payload.str))
	dst := C.GoStringN(query.payload.str, C.int(query.payload.length))
	DPrintf("dst IP: %v, Port: %v", dst, int(query.destination_port))
	dstConn, err := net.Dial("tcp", dst+":"+strconv.Itoa(int(query.destination_port)))
	if err != nil {
		return nil, err
	}
	ip := dstConn.RemoteAddr().(*net.TCPAddr).IP.To16()
	if ip == nil {
		return nil, errors.New("Can't convert IP!")
	}
	var queryFrom C.client_query
	queryFrom.destination_port = query.destination_port
	for id, data := range ip {
		queryFrom.destination_ip[id] = C.uint8_t(data)
	}
	queryFrom.payload.length = 0
	dataFrom := C.call_server_encode(&connectInfo, queryFrom)
	defer C.free(unsafe.Pointer(dataFrom.str))
	DPrintf("Response to DNS Query: length %v",dataFrom.length)
	ok = writeBinarySafeString(client, dataFrom)
	if !ok {
		return nil, errors.New("get address error!")
	}
	return dstConn, nil
}

func read(cliConn net.Conn, connectInfo C.connect_info, dstConn net.Conn, c chan bool) {
	for {
		if !readEach(cliConn, connectInfo, dstConn) {
			c<-true
			return
		}
	}
	return
}

func readEach(cliConn net.Conn, connectInfo C.connect_info, dstConn net.Conn) bool {
	var dataFrom C.binary_safe_string
	ok := readBinarySafeString(cliConn, &dataFrom)
	if !ok {
		return false
	}
	query := C.call_server_decode(&connectInfo, dataFrom)
	defer C.free(unsafe.Pointer(query.payload.str))
	//DPrintf("query from client: %v", query)
	_, err := dstConn.Write([]byte(C.GoStringN(query.payload.str, C.int(query.payload.length))))
	if err != nil {
		if err != io.EOF {
			EPrintf("%v", err)
		}
		return false
	}
	return true
}

func write(cliConn net.Conn, connectInfo C.connect_info, dstConn net.Conn, c chan bool) {
	for {
		if !writeEach(cliConn, connectInfo, dstConn) {
			c<-true
			return
		}
	}
	return
}

func writeEach(cliConn net.Conn, connectInfo C.connect_info, dstConn net.Conn) bool {
	var query C.client_query
	load := make([]byte, 1024)
	lens, err := dstConn.Read(load)
	if err != nil {
		if err != io.EOF {
			DPrintf("Err: %v", err)
		}
		return false
	}
	//DPrintf("load from destination: %v", string(load))
	query.payload.length = C.uint32_t(lens)
	query.payload.str = C.CString(string(load[:lens]))
	remoteAddr := dstConn.RemoteAddr().(*net.TCPAddr)
	ip := remoteAddr.IP.To16()
	if ip == nil {
		DPrintf("IP convert error!")
		C.free(unsafe.Pointer(query.payload.str))
		return false
	} else {
		DPrintf("IP converted: %v", ip)
	}
	for index, c := range ip {
		query.destination_ip[index] = C.uint8_t(c)
	}
	DPrintf("Send to server: %v", query.destination_ip)
	portB := make([]byte, 2)
	binary.LittleEndian.PutUint16(portB, uint16(remoteAddr.Port))
	query.destination_port = C.uint16_t(binary.BigEndian.Uint16(portB))
	dataBack := C.call_server_encode(&connectInfo, query)
	defer C.free(unsafe.Pointer(dataBack.str))
	ok := writeBinarySafeString(cliConn, dataBack)
	if !ok {
		return false
	} else {
		return true
	}
}
