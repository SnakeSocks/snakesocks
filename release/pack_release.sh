#!/bin/bash

[[ $2 == '' ]] && echo 'Usage: ./release.sh <version> <arch>' && exit 2
ver=$1
arch=$2
cd ..
make clean
# WORKDIR = project root dir

# Platform-independent files.
rel=release/snakesocks-prebuilt-linux-$ver
if [[ ! -d $rel ]]; then
    # Init the release directory
    cp -r release/template $rel || exit $?
fi
cp src/native/server-example.conf $rel/server.conf &&
cp src/native/client-example.conf $rel/client.conf &&
cp README.md $rel/ || exit $?

# Platform-dependent files.
rel=$rel/$arch
[[ -d $rel ]] && echo "Directory $rel already exists." && exit 1
mkdir -p $rel/mods || exit $?

make native &&
cp src/native/skcli $rel/ &&
cp src/native/sksrv $rel/ &&
cd src/modules && make se_proxy && cd - && 
cp src/modules/se_proxy.so $rel/mods/ &&
echo 'done.' || (echo 'failed.' ; exit 1)

