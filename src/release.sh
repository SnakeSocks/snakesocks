#!/bin/bash

[[ $2 == '' ]] && echo 'Usage: ./release.sh <version> <arch>' && exit 2
ver=$1
arch=$2
cd ..

rel=release/$ver/$arch
mkdir -p $rel/mod > /dev/null 2>&1

make client && make server &&
cp src/client/skcli $rel/ &&
cp src/server/sksrv $rel/ &&
cp src/client/example.conf $rel/client.conf &&
cp src/server/example.conf $rel/server.conf &&
cd src/modules && make se_proxy && cd - && 
cp src/modules/se_proxy.so $rel/mods/ &&
echo 'ok. copy install/uninstall.sh manually' ||
echo 'failed.'


