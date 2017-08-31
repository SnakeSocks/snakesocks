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
sudo make install
```

- server

```sh
git clone https://github.com/snakesocks/snakesocks.git
cd snakesocks
make server
sudo make install
```

- client for windows (mingw)

Building mingw edition from source is **not** recommended. Usually it's necessary to edit CMakeLists_win.txt and rename it to CMakeLists.txt. Then try launching your build:

```sh
git clone https://github.com/snakesocks/snakesocks.git && cd snakesocks/src/client
vim CMakeLists_win.txt # Set some env var.
mv CMakeLists_win.txt CMakeLists.txt
cmake -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Release . && make
```

Then you'll get skcli.exe.

## Install and use modules

[TODO]

## Build your module from source

[TODO]

## Write your own module

## Configuration

SnakeSocks has its configuration files in `/etc/snakesocks/conf`. Just edit `client.conf` or `server.conf` to fit your demand.

The client and the server should use the same module, unless you're really clear about what you're doing.
