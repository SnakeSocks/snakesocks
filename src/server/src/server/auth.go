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
	"unsafe"
)

func authClient(client net.Conn, connectInfo *C.connect_info) bool {
	remoteAddr := client.RemoteAddr().(*net.TCPAddr)
	ip := remoteAddr.IP.To16()
	if ip == nil {
		EPrintf("(%v) IP convert Error!", remoteAddr)
		return false
	}
	port := uint16(remoteAddr.Port)
	for index, c := range ip {
		connectInfo.server_ip[index] = C.uint8_t(c)
	}
	connectInfo.server_port = C.uint16_t(port)
	connectInfo.connect_fd = -1
	//TODO:client connect fd mustn't be used for anything now!
	var auth C.binary_safe_string
	readBinarySafeString(client, &auth)
	var isClosing bool
	is_closing := C.bool(isClosing)
	readyToSend := C.call_server_make_auth_reply(connectInfo, auth, &is_closing)
	writeBinarySafeString(client, readyToSend)
	if isClosing {
		C.free(unsafe.Pointer(readyToSend.str))
		return false
	} else {
		return true
	}
}
