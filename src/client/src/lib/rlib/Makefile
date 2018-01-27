CXX ?= g++
CC ?= gcc
AR ?= ar
CXXFLAGS = -O3
CFLAGS = 
ARFLAGS = rcs

def: compile_library

compile_library:
	$(CXX) $(CXXFLAGS) -c libr.cc -o libr.o
	$(AR) $(ARFLAGS) libr.a libr.o

install_header:
	[ ! -d /usr/include/rlib ] || rm -rf /usr/include/rlib
	cp -r . /usr/include/rlib
	rm -rf /usr/include/rlib/test /usr/include/rlib/.git

install_library: compile_library
	cp libr.a /usr/lib/

install: install_header install_library

uninstall:
	rm -rf /usr/include/rlib
	rm /usr/lib/libr.a

clean:
	rm *.o *.a
