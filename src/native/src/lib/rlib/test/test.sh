#!/bin/bash

for cxx in g++ clang++ icpc
do
    for std in 14 17
    do
        echo "Testing $cxx c++$std..."
        make CXX="$cxx" STD="$std"
        [[ $? != 0 ]] && echo "Testing $cxx c++$std failed. Exiting..." && exit 1
    done
done

echo "All tests passed."


