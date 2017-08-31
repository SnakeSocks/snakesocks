# SnakeSocks [![Build Status](https://travis-ci.org/SnakeSocks/snakesocks.svg?branch=master)](https://travis-ci.org/SnakeSocks/snakesocks)

SnakeSocks is a useful proxy for exchanges network packets between a client and server through firewall or other things. 

Due to our test, it may have less latency but smaller bandwidth compared with sha*owsocks.

## Dependencies

- Runtime

glibc 2.14

- Compilation

C++14 support and C++ Boost Library (client), Golang (server)

## Install from release

Download and unpack snakesocks from [here](https://github.com/snakesocks/snakesocks/releases), then run `./install.sh client/server`.

## Build from source

- client
```sh
git clone https://github.com/snakesocks/snakesocks
cd snakesocks
make client
sudo make install
```
- server
```sh
git clone https://github.com/snakesocks/snakesocks
cd snakesocks
make server
sudo make install
```
## Install and use modules

[TODO]

## Build your module from source

[TODO]

## Write your own module

## Configuration

SnakeSocks has its configuration files in `/etc/snakesocks/conf`. Just edit `client.conf` or `server.conf` to fit your demand.

The client and the server should use the same module, unless you're really clear about what you're doing.
