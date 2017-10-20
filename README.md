# SnakeSocks [![Build Status](https://travis-ci.org/SnakeSocks/snakesocks.svg?branch=master)](https://travis-ci.org/SnakeSocks/snakesocks)

SnakeSocks is a useful proxy for exchanges network packets between a client and server through firewall or other things. 

Due to our test, it may have less latency but smaller bandwidth compared with sha*owsocks.

Tip: 'client' is 'client for linux/unix' if there's no special instructions.

## Dependencies

- Runtime

glibc 2.14

- Compilation

C++14 support and C++ Boost Library (client), Golang (server)

## Install from release

Download and unpack snakesocks from [here](https://github.com/snakesocks/snakesocks/releases), then run `./install.sh client/server`.

Client for windows will be released together with snakesocks-windows-gui, at [here](https://github.com/snakesocks/snakesocks-windows).

## Build from source

- client

```sh
git clone https://github.com/snakesocks/snakesocks.git
cd snakesocks
make client
make default_modules
sudo make install
skcli -h
```

- server

```sh
git clone https://github.com/snakesocks/snakesocks.git
cd snakesocks
make server
make default_modules
sudo make install
sksrv -h
```

- client for windows (mingw)

Refer to [here](https://github.com/snakesocks/snakesocks-windows) please.

## Install and use modules

1. copy the .so module file to `/etc/snakesocks/modules/` or wherever you like.
2. change the corresponding configuration to the path of the .so file.
3. make sure the server and the client use the same module.

## Build your module from source

```shell
cd src/modules
make <module_name>
# and the corresponding .so file will be generated.
# omit <module_name> will build the default destination.
```

## Write your own module

### Develop

1. Read `stdafx.h` `conn.h` `bss.h` `client_query.h` for specifications of module APIs.
2. Read `simple_proxy` and `se_proxy` implementation for examples of module implementation.
3. Write your own module.

### Test

```shell
# first,modify Makefile to add your module builds
# follow examples of se_proxy and simple_proxy
# then, modify config.mk to set your default build destination.
# then run
make test
./test
# to run the test program we provided to test the module you write
```

## Configuration

SnakeSocks has its configuration files in `/etc/snakesocks/conf`. Just edit `client.conf` or `server.conf` to fit your demand.

The client and the server should use the same module, unless you're really clear about what you're doing.

## Optimize

Usually, tcpbbr or kcptun is useful to improve connection stability.

