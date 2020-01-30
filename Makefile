nofakeroot_prefix=/etc/snakesocks
prefix=$$pkgdir$(nofakeroot_prefix)

def:
	echo 'Usage: "make native" or "make server-go" or "make default_modules"' && exit 1

cmake_extra_arg :=
check_cc_env:
ifdef SKCLI_CC
cmake_extra_arg += -DCMAKE_C_COMPILER=$(SKCLI_CC)
endif
ifdef SKCLI_CXX
cmake_extra_arg += -DCMAKE_CXX_COMPILER=$(SKCLI_CXX)
endif

native: check_cc_env
	cd src/native && cmake . -DCMAKE_BUILD_TYPE=Release $(cmake_extra_arg) && $(MAKE) && cd -

client: native

server: native

server-go:
	cd src/server-go && $(MAKE) skserver && cd -

default_modules:
	cd src/modules && make simple_proxy && make se_proxy && cd -

init_dir:
	mkdir -p $(prefix)/modules; mkdir -p $(prefix)/conf; mkdir -p $$pkgdir/usr/bin

install: init_dir
	[ -f src/native/sksrv ] && cp src/native/sksrv $(prefix)/sksrv && CONF_NAME='server.conf' && [ -f $(prefix)/conf/server.conf ] && CONF_NAME="$$CONF_NAME"'.new' || echo -n '' && [ -f src/native/sksrv ] && cp src/native/server-example.conf $(prefix)/conf/$$CONF_NAME && $$([ -L $$pkgdir/usr/bin/sksrv ] || ln -s $(nofakeroot_prefix)/sksrv $$pkgdir/usr/bin/sksrv) || echo -n ''
	[ -f src/native/skcli ] && cp src/native/skcli $(prefix)/skcli && CONF_NAME='client.conf' && [ -f $(prefix)/conf/client.conf ] && CONF_NAME="$$CONF_NAME"'.new' || echo -n '' && [ -f src/native/skcli ] && cp src/native/client-example.conf $(prefix)/conf/$$CONF_NAME && $$([ -L $$pkgdir/usr/bin/skcli ] || ln -s $(nofakeroot_prefix)/skcli $$pkgdir/usr/bin/skcli) || echo -n ''
	cp src/modules/*.so $(prefix)/modules/ > /dev/null 2>&1 || echo -n '' # Urgent temporary fix.

uninstall:
	rm -rf /etc/snakesocks /usr/bin/skcli /usr/bin/sksrv

clean:
	cd src/server-go && $(MAKE) clean && cd -
	cd src/native && ./cmake_clean.sh && cd -
	cd src/modules && make clean && cd -
