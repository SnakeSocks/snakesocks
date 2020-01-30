CXX ?= g++
CC ?= gcc
AR ?= ar
CXXFLAGS = -O3 -std=c++14 -fPIC
CFLAGS = 
ARFLAGS = rcs

PREFIX ?= /usr

def:
	@echo Run make install

install_header:
	[ ! -d $(PREFIX)/include/rlib ] || rm -rf $(PREFIX)/include/rlib
	cp -r . $(PREFIX)/include/rlib
	rm -rf $(PREFIX)/include/rlib/test $(PREFIX)/include/rlib/.git

install_cmake:
	[ ! -d $(PREFIX)/lib/cmake/rlib ] || rm -rf $(PREFIX)/lib/cmake/rlib
	[ ! -d $(PREFIX)/lib/cmake ] || cp -r cmake $(PREFIX)/lib/cmake/rlib

install: install_header install_cmake

uninstall:
	rm -rf $(PREFIX)/include/rlib $(PREFIX)/lib/cmake/rlib

clean:

.PHONY: test

test:
	cd test && ./test.sh

