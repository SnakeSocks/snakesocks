package server

/*
#include "stdafx.h"
#include "stdlib.h"
#include "dlf.h"

binary_safe_string call_server_make_auth_reply(connect_info * conf, binary_safe_string bss, bool * is_closing){
	return f_server_make_auth_reply(conf,bss,is_closing);
}

client_query call_server_decode(connect_info * conf,binary_safe_string data){
	return f_server_decode(conf,data);
}

binary_safe_string call_server_encode(connect_info *conf, client_query payload){
	return f_server_encode(conf,payload);
}

void call_server_connection_close(connect_info *conf){
	return f_server_connection_close(conf);
}

#cgo LDFLAGS: -ldl
*/
import "C"

import (
	"io"
	"net"
	"encoding/binary"
	"unsafe"
	"log"
	"errors"
	"strconv"
)

type BssRW struct {
	net.Conn
}

func (brw *BssRW) ReadB(data *C.binary_safe_string) error {
	header := make([]byte, 4)
	if n, err := io.ReadFull(brw, header[:]); n != 4 || err != nil {
		if err != nil {
			return err
		} else {
			return errors.New("Reading " + strconv.Itoa(n) + "!=" + strconv.Itoa(int(4)) + " bytes")
		}
	}
	size := binary.BigEndian.Uint32(header)
	if size > 10240000 {
		EPrintf("Header : %v", header)
		EPrintf("Size : %v", size)
		PPrintf(errors.New("out of memory(by user)"))
	}
	packet := make([]byte, size)
	if n, err := io.ReadFull(brw, packet[:]); n != int(size) || err != nil {
		if err != nil {
			return err
		} else {
			return errors.New("Reading " + strconv.Itoa(n) + "!=" + strconv.Itoa(int(size)) + " bytes")
		}
	}
	data.length = C.uint32_t(size)
	data.str = C.CString(string(packet))
	//DPrintf("[BssRW]: Read: %v",string(packet))
	return nil
}

func (brw *BssRW) WriteB(data C.binary_safe_string) error {
	defer C.free(unsafe.Pointer(data.str))
	dataHeader := make([]byte, 4)
	binary.BigEndian.PutUint32(dataHeader, uint32(data.length))
	dataBody := []byte(C.GoBytes(unsafe.Pointer(data.str), C.int(data.length)))
	_, err := brw.Write(dataHeader)
	//DPrintf("[BssRW]: Write: %v",string(dataB))
	if err != nil {
		return err
	}
	_, err = brw.Write(dataBody)
	if err != nil {
		return err
	}
	return nil
}

func DPrintf(format string, a ...interface{}) (n int, err error) {
	if Debug > 2 {
		log.Printf(format, a...)
	}
	return
}

func EPrintf(format string, a ...interface{}) (n int, err error) {
	if Debug > 1 {
		log.Printf("Error :"+format, a...)
	}
	return
}

func PPrintf(err error) {
	if Debug > 0 {
		log.Panic(err)
	}
	return
}
