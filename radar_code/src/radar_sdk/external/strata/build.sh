#!/bin/bash

set -e

if [ "$OS" = "Windows_NT" ]
    then PATH=/c/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin:$PATH
fi

SOURCE_DIR=${PWD##*/}
BUILD_DIR=$SOURCE_DIR-build

echo Building \"$SOURCE_DIR\" in \"$BUILD_DIR\" ...

mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DSTRATA_BUILD_SAMPLES:BOOL=OFF -DSTRATA_BUILD_TESTS:BOOL=ON -DSTRATA_BUILD_TOOLS:BOOL=OFF ../. -G"Unix Makefiles"

make $@
if [ $? = 0 ]
    then make install
fi

if [ "$OS" = "Windows_NT" ]
then
    read -N 1 -p "Press any key to continue . . . "
fi
