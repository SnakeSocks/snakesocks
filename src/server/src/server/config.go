package server

import (
	"flag"
	"github.com/go-ini/ini"
	"fmt"
	"os"
)

var Debug int
var DylibPath string
var ConfigPath string
var Port int
var Passphrase string

func initConfig() error {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "SnakeSocks Server 1.3.1\n\n")
		flag.PrintDefaults()
		fmt.Fprintf(os.Stderr, "\nPublished on GNU license V2.\n")
	}
	flag.StringVar(&ConfigPath, "c", "/etc/snakesocks/conf/server.conf", "proxy config path")
	flag.IntVar(&Port, "p", 8080, "server monitor port")
	flag.IntVar(&Debug, "d", 2, "debug level(1,2,3)")
	flag.StringVar(&DylibPath, "l", "/etc/snakesocks/modules/sample.so", "dynamic library path")
	flag.StringVar(&Passphrase, "", "", "the passphrase for encypt")
	flag.Parse()

	cfg, err := ini.Load(ConfigPath)
	if err != nil {
		return err
	}
	core := cfg.Section("core")
	extra := cfg.Section("extra")
	if DylibPath == "/etc/snakesocks/modules/sample.so" {
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
