#!/bin/bash

function assert () {
    CMD="$*"
    $CMD
    RET_VAL=$?
    if [ $RET_VAL != 0 ]; then
        echo "Assertion failed: $CMD returns $RET_VAL."
        exit $RET_VAL
    fi
}

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 2
fi

[[ $1 == '' ]] && echo 'usage: ./install.sh server or ./install.sh client' && exit 1

arch=`uname -m`
[[ ! -d $arch ]] && echo "Your arch is $arch but prebuilt binary (in directory './$arch') is not existing. Please build from source." && exit 3

# Working around for ABS makepkg (AUR).
[[ ! $pkgdir == '' ]] && echo 'Running install.sh for ABS makepkg. If this is not what you are willing, do not set variable `pkgdir`!'
nofakeroot_prefix='/etc/snakesocks'
[[ $prefix == '' ]] && prefix="$pkgdir$nofakeroot_prefix"

# Launch installation
mkdir -p $pkgdir/usr/bin > /dev/null 2>&1
if [ "$1" == "server" ]; then
    echo 'Installing server...'
    assert mkdir -p $prefix/conf
    if [ -f "$prefix/conf/server.conf" ]; then
        conf_target="$prefix/conf/server.conf.upgrade"
        echo 'server.conf already exists. Install as server.conf.upgrade'
    else
        conf_target="$prefix/conf/server.conf"
    fi
    assert cp server.conf $conf_target
    assert cp $arch/sksrv $prefix/sksrv
    [[ ! -L $pkgdir/usr/bin/sksrv ]] && assert ln -s $nofakeroot_prefix/sksrv $pkgdir/usr/bin/sksrv

    # Systemd service
    if [ -d /etc/systemd/system ]; then
        echo 'Installing systemd service `snakesocks-server`...'
        assert cp snakesocks-server.service /etc/systemd/system/
    fi
elif [ "$1" == "client" ]; then
    echo 'Installing client...'
    assert mkdir -p $prefix/conf
    if [ -f "$prefix/conf/client.conf" ]; then
        conf_target="$prefix/conf/client.conf.upgrade"
        echo 'client.conf already exists. Install as client.conf.upgrade'
    else
        conf_target="$prefix/conf/client.conf"
    fi
    assert cp client.conf $conf_target
    assert cp $arch/skcli $prefix/skcli
    [[ ! -L $pkgdir/usr/bin/skcli ]] && assert ln -s $nofakeroot_prefix/skcli $pkgdir/usr/bin/skcli

    # Systemd service
    if [ -d /etc/systemd/system ]; then
        echo 'Installing systemd service `snakesocks-client`...'
        assert cp snakesocks-client.service /etc/systemd/system/
    fi

    # Fish auto-complete.
    grep '/usr/bin/fish' /etc/shells && 
        echo 'Trying to install skcli auto-completion script for fish...' && mkdir -p ~/.config/fish/completions && cp skcli-completion.fish ~/.config/fish/completions/
else
    echo "No target '$1' to install."
    exit 1
fi

echo 'Installing pre-compiled modules...'
assert mkdir -p $prefix/modules
assert cp $arch/mods/*.so $prefix/modules/
