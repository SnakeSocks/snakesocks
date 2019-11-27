#FROM archlinux/base
FROM ubuntu

MAINTAINER snakesocks@mail.com

RUN apt update && apt install -y gcc g++ gccgo-go libboost-system-dev cmake make

RUN mkdir /app
COPY . /app
WORKDIR /app

RUN make clean client server default_modules install



