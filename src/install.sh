#!/bin/bash

if [ $# -ne 2 ]; then
    echo "usage : ./$0 <what_you_want_to_install> <module_you_want_to_load>"
    echo "    what_you_want_to_install : \"server\" or \"client\""
    echo "    module_you_want_to_load : the module you want to load to client or server."
    echo "                              will only search from modules folder."
    echo "example:"
    echo "    ./$0 server my_module --> will search \"my_module.so\" in modules folder and install it with server."
    exit
fi

echo " -- Performing install for $1.."

sh $1/install.sh

if [ -e modules/$2.so ]; then
    echo " -- Performing module install for $1: the module selected is $2."
    sh $1/install_module.sh modules/$2.so

    echo " -- Installation procedure completed."
else

    echo " -- Specified module not found."
    echo " -- Installation procedure terminated. Modules not installed."
    echo " -- Manually install a module before running."

fi
