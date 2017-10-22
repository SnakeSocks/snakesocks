
#CC?=gcc
TESTS := $(shell find test -name '*.c')

build:
	$(foreach fl, $(TESTS), $(CC) -c $(fl) -o /tmp/test.o;)

install:
	[ ! -d /usr/include/rlib ] || rm -rf /usr/include/rlib
	cp -r . /usr/include/rlib
	rm -rf /usr/include/rlib/test /usr/include/rlib/.git

uninstall:
	rm -rf /usr/include/rlib
