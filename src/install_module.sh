#!/bin/bash

if [ $# -ne 2 ]; then
    echo "usage: ./$0 <program_you_have_installed> <the_module_you_want_to_install>"
    echo "    this program will perform module installation/replacement."
    echo "    provided module name will only be automatically searched in \"modules\" folder"
    echo "    like install.sh does."
    exit
fi
if [ -e modules/$2.so ]
    echo " -- Performing module installation for $1: selected module is $2.so .."
    sh $1/load_module.sh `pwd`/modules/$2.so
    echo " -- Module installation for $2.so completed."
else
    echo " -- Module specified not found."
    echo " -- Procedure terminated."
fi