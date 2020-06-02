#!/bin/sh
make clean && make CXX="arm-linux-gnueabihf-g++" EXTRA_CXXFLAGS="-mcpu=cortex-a7 -mfpu=neon" LIBNAME="supafaust-arm32-cortex-a7.so" -j4 && \
make clean && make CXX="arm-linux-gnueabihf-g++" EXTRA_CXXFLAGS="-mcpu=cortex-a9 -mfpu=neon" LIBNAME="supafaust-arm32-cortex-a9.so" -j4 && \
make clean && make CXX="arm-linux-gnueabihf-g++" EXTRA_CXXFLAGS="-mcpu=cortex-a15 -mfpu=neon" LIBNAME="supafaust-arm32-cortex-a15.so" -j4 && \
make clean && make CXX="arm-linux-gnueabihf-g++" EXTRA_CXXFLAGS="-mcpu=cortex-a53 -mfpu=neon" LIBNAME="supafaust-arm32-cortex-a53.so" -j4 && \
make clean && make CXX="aarch64-linux-gnu-g++" EXTRA_CXXFLAGS="-mcpu=cortex-a53" LIBNAME="supafaust-arm64.so" -j4 && \
true
