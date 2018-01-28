nofakeroot_prefix=/etc/snakesocks
prefix=$$pkgdir$(nofakeroot_prefix)

def:
	echo 'Usage: "make server" or "make client" or "make default_modules"' && exit 1

cmake_extra_arg :=
check_cc_env:
ifdef SKCLI_CC
cmake_extra_arg += -DCMAKE_C_COMPILER=$(SKCLI_CC)
endif
ifdef SKCLI_CXX
cmake_extra_arg += -DCMAKE_CXX_COMPILER=$(SKCLI_CXX)
endif

server:
	cd src/server && $(MAKE) skserver && cd -

client: check_cc_env
	cd src/client && cmake . -DCMAKE_BUILD_TYPE=Release $(cmake_extra_arg) && $(MAKE) && cd -

default_modules:
	cd src/modules && make simple_proxy && make se_proxy && cd -

init_dir:
	mkdir -p $(prefix)/modules; mkdir -p $(prefix)/conf; mkdir -p $$pkgdir/usr/bin

install: init_dir
	[ -f src/server/sksrv ] && cp src/server/sksrv $(prefix)/sksrv && CONF_NAME='server.conf' && [ -f $(prefix)/conf/server.conf ] && CONF_NAME="$$CONF_NAME"'.new' || echo -n '' && [ -f src/server/sksrv ] && cp src/server/example.conf $(prefix)/conf/$$CONF_NAME && $$([ -L $$pkgdir/usr/bin/sksrv ] || ln -s $(nofakeroot_prefix)/sksrv $$pkgdir/usr/bin/sksrv) || echo -n ''
	[ -f src/client/skcli ] && cp src/client/skcli $(prefix)/skcli && CONF_NAME='client.conf' && [ -f $(prefix)/conf/client.conf ] && CONF_NAME="$$CONF_NAME"'.new' || echo -n '' && [ -f src/client/skcli ] && cp src/client/example.conf $(prefix)/conf/$$CONF_NAME && $$([ -L $$pkgdir/usr/bin/skcli ] || ln -s $(nofakeroot_prefix)/skcli $$pkgdir/usr/bin/skcli) || echo -n ''
	cp src/modules/*.so $(prefix)/modules/ > /dev/null 2>&1 || echo -n '' # Urgent temporary fix.

uninstall:
	rm -rf /etc/snakesocks /usr/bin/skcli /usr/bin/sksrv

clean:
	cd src/server && $(MAKE) clean && cd -
	cd src/client && ./cmake_clean.sh && cd -
	cd src/modules && make clean && cd -
