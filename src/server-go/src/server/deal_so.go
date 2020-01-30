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

void initSo(const char *filePath){
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

void closeSo(){
	if(handle) dlclose(handle);
}
#cgo LDFLAGS: -ldl
*/
import "C"

func loadSo(path string) {
	C.initSo(C.CString(path))
}

func closeSo() {
	C.closeSo()
}
