#!/bin/bash

build() {
  CURRENT=$(dirname $(readlink -f "$0"))

  PLATFORM_ABI=$1
  case $PLATFORM_ABI in
    "x86_64")
      TOOLCHAIN_FILE="$CURRENT/toolchain/x86_64.toolchain.cmake"
      ;;
  esac

  rm -rf build/$PLATFORM_ABI
  cmake -S ./src -B ./build/$PLATFORM_ABI \
    -DSDK_INCLUDE_PATH="$CURRENT/../../../Release/linux/include" \
    -DSDK_LIB_PATH="$CURRENT/../../../Release/linux/lib/$PLATFORM_ABI" \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$CURRENT/bin"
  cd build/$PLATFORM_ABI && make -j8 || exit 1
}

build x86_64
