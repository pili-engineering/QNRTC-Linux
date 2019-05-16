#/bin/sh
ARCH=linux
if test -d build.${ARCH}
then rm -rf build.${ARCH}
fi
mkdir build.${ARCH}

cd build.${ARCH}
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake_toolchain/${ARCH}.std ..
make

cp -f demo ../../Release/lib/${ARCH}/

