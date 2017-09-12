package server

/*
#include "stdafx.h"
#include "stdlib.h"
#include "dlf.h"

_f_server_make_auth_reply_t f_server_make_auth_reply;
_f_server_encode_t f_server_encode;
_f_server_decode_t f_server_decode;
_f_server_connection_close_t f_server_connection_close;

void *handle = NULL;

void initSo(const char *filePath)
{
	handle = dlopen(filePath, RTLD_LAZY);
    char *errstr = NULL;
    if(!handle) {
        printf("dlopen failed to open shared object %s, dlerror=%s", filePath, dlerror());
        exit(3);
    }
    dlerror(); //Clear

    f_server_encode = dlsym(handle, "server_encode");
    if ((errstr = dlerror()) != NULL) {
        printf("find symbol server_encode from %s failed, dlerror=%s", filePath, dlerror());
        exit(3);
    }
    f_server_decode = dlsym(handle, "server_decode");
    if ((errstr = dlerror()) != NULL) {
        printf("find symbol server_decode from %s failed, dlerror=%s", filePath, dlerror());
        exit(3);
    }
    f_server_make_auth_reply = dlsym(handle, "server_make_auth_reply");
    if ((errstr = dlerror()) != NULL) {
        printf("find symbol server_make_auth_reply from %s failed, dlerror=%s", filePath, dlerror());
        exit(3);
    }
    f_server_connection_close = dlsym(handle, "server_connection_close");
    if ((errstr = dlerror()) != NULL) {
        printf("find symbol server_connection_close from %s failed, dlerror=%s", filePath, dlerror());
        exit(3);
    }

}

void closeSo()
{
	if(handle) dlclose(handle);
}
#cgo LDFLAGS: -ldl
*/
import "C"

import (
	"net"
	"strconv"
	"unsafe"
)

var cnt int
func StartServer() {
	err := initConfig()
	cnt = 0
	if err != nil {
		PPrintf(err)
	}
	C.initSo(C.CString(DylibPath))
	l, err := net.Listen("tcp", ":"+strconv.Itoa(Port))
	if err != nil {
		PPrintf(err)
	}

	for {
		client, err := l.Accept()
		cnt++
		DPrintf("---Client open files:%v---",cnt)
		if err != nil {
			EPrintf("Err: %v",err)
		}
		go handleClientRequest(client)
	}
	C.closeSo()
}

func handleClientRequest(client net.Conn) {
	if client == nil {
		return
	}
	DPrintf("Client with address(%v) connected to server", client.RemoteAddr())
	DPrintf("Start to auth the client")
	var connectInfo C.connect_info
	var pass C.binary_safe_string
	pass.null_terminated = false
	pass.length = C.uint32_t(len(Passphrase))
	pass.str = C.CString(Passphrase)
	defer C.free(unsafe.Pointer(connectInfo.passphrase.str))
	connectInfo.passphrase = pass
	if ok := authClient(client, &connectInfo); !ok {
		DPrintf("Auth false")
	}else{
		DPrintf("Auth true")
		proxyGo(client, connectInfo)
	}
	client.Close()
	cnt--
	DPrintf("---Client open files: %v---",cnt)
	return
}
