# Compile and run tests for rlib
# CopyRight (C) 2017-2018 Recolic Keghart <root@recolic.net>
#
# Tests may fail on both compile-time(traits and meta-lib) and run-time(return non-zero).
# Use `make <module>` to build and run a module, 
# 	and `make` to build and run all modules.
# 
# ready-to-use modules:
# 	string      rlib/string.hpp
# 	meta        rlib/meta.hpp
# 	trait       rlib/traits.hpp
# 	stdio       rlib/stdio.hpp
# 	scope_guard rlib/scope_guard.hpp rlib/scope_guard_buffer.hpp
# 	opt         rlib/opt.hpp
# 	sio         rlib/sys/sio.hpp
# 	os          rlib/sys/os.hpp
# 	require     rlib/require/*.hpp

# Test dependency:
# string ---> common 

MODULES=string meta trait stdio sio scope_guard

XTRA_FLAGS ?=
CXXFLAGS=-I. -I../.. $(XTRA_FLAGS)
STD ?= 14
FLAGS11=-std=c++11
FLAGS14=-std=c++14
FLAGS17=-std=c++17

ifeq ($(STD),11)
	CXXFLAGS := $(CXXFLAGS) $(FLAGS11)
endif
ifeq ($(STD),14)
	CXXFLAGS := $(CXXFLAGS) $(FLAGS14)
endif
ifeq ($(STD),17)
	CXXFLAGS := $(CXXFLAGS) $(FLAGS17)
endif

POSTFIX=$(STD)_$(CXX)

all: string common

common:
	$(CXX) $(CXXFLAGS) src/common.cc $(CXXFLAGS) -o src/common_$(POSTFIX).out
	src/common_$(POSTFIX).out

string:
	$(CXX) $(CXXFLAGS) src/string.cc $(CXXFLAGS) -o src/string_$(POSTFIX).out
	src/string_$(POSTFIX).out

clean:
	rm -f src/*.out


