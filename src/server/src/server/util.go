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
	"encoding/binary"
	"flag"
	"fmt"
	"github.com/go-ini/ini"
	"log"
	"net"
	"os"
	"io"
	"unsafe"
)

var Debug int
var DylibPath string
var ConfigPath string
var Port int
var Passphrase string

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

func initConfig() error {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "SnakeSocks Server 1.2\n\n")
		flag.PrintDefaults()
		fmt.Fprintf(os.Stderr, "\nPublished on GNU license V2.\n")
	}
	flag.StringVar(&ConfigPath, "c", "/etc/snakesocks/conf/server.conf", "proxy config path")
	flag.IntVar(&Port, "p", 8080, "server monitor Port")
	flag.IntVar(&Debug, "d", 2, "debug level(1,2,3)")
	flag.StringVar(&DylibPath, "l", "modules/libmymodule.so", "dynamic library path")
	flag.StringVar(&Passphrase, "", "", "the Passphrase for encypt")
	flag.Parse()

	cfg, err := ini.Load(ConfigPath)
	if err != nil {
		return err
	}
	core := cfg.Section("core")
	extra := cfg.Section("extra")
	if DylibPath == "modules/libmymodule.so" {
		DylibPath = core.Key("module").String()
	}
	if Port == 8080 {
		Port, err = core.Key("port").Int()
		if err != nil {
			return err
		}
	}
	if Debug == 2 {
		Debug, err = extra.Key("debugLevel").Int()
		if err != nil {
			return err
		}
	}
	if Passphrase == "" {
		Passphrase = core.Key("passphrase").String()
	}

	return nil
}

func readBinarySafeString(conn net.Conn, data *C.binary_safe_string) bool {
	headers, err := readN(conn, 4)
	if err != nil {
		if err != io.EOF {
			DPrintf("Err: %v", err)
		}
		return false
	}
	//dataNullTerminated := headers[0] == 1
	//data.null_terminated = C.bool(dataNullTerminated)
	dataLength := binary.BigEndian.Uint32(headers)
	DPrintf("headers: %v to datalength : %v", headers, dataLength)
	data.length = C.uint32_t(dataLength)
	if dataLength >= 1000000 {
		DPrintf("Boooooooom!!!!!!!!")
		return false
	}
	content, err := readN(conn, int(dataLength))
	if err != nil {
		DPrintf("Err: %v", err)
		return false
	}
	DPrintf("data: %v", content)
	data.str = C.CString(string(content))
	return true
}

func writeBinarySafeString(conn net.Conn, data C.binary_safe_string) bool {
	dataB := make([]byte, 4)
	binary.BigEndian.PutUint32(dataB, uint32(data.length))
	dataB = append(dataB, []byte(C.GoBytes(unsafe.Pointer(data.str), C.int(data.length)))...)
	_, err := conn.Write(dataB[:])
	DPrintf("Write binary safe string size %v:%v", uint32(data.length), dataB)
	if err != nil {
		if err != io.EOF {
			DPrintf("Err: %v", err)
		}
		return false
	}
	return true
}

func readN(conn net.Conn, n int) ([]byte, error) {
	dataB := make([]byte, n)
	nowNum := 0
	for nowNum < n {
		readNum, readErr := conn.Read(dataB[nowNum:])
		if readErr != nil {
			return nil, readErr
		}
		nowNum += readNum
	}
	return dataB, nil
}
