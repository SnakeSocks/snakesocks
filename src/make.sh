#!/bin/bash

echo " -- Configurating client build parameters.."

sh client/config.sh

echo " -- Configurating server build parameters.."

sh server/config.sh

echo " -- Configurating modules build parameters.."

sh modules/config.sh

echo " -- Start building server.."

cd server

make

echo " -- Leaving server directory"

cd ..

echo " -- Start building client.."

cd client

make

echo " -- Leaving client directory"

cd ..

echo " -- Start building modules.."

cd modules

make

echo " -- Leaving client directory"

cd ..

echo " -- Make procedure over."