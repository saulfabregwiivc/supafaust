#!/bin/sh
make clean && make CXX="g++" EXTRA_CXXFLAGS="-mtune=atom -fno-PIC -fpic -mcmodel=small" LIBNAME="supafaust-x86_64.so" -j4 && \
true

# TODO: need to set up proper toolchains
#make clean && make CXX="g++" EXTRA_CXXFLAGS="-march=pentium3 -mtune=atom -fno-PIC -fno-pic" LIBNAME="supafaust-x86.so" -j4 && \
#make clean && make CXX="i686-w64-mingw32-g++" EXTRA_CXXFLAGS="-march=pentium3 -mtune=atom -fno-PIC -fno-pic" LIBS="-static -lwinmm" OBJ_PLATFORM="mednafen/mthreading/MThreading_Win32.o mednafen/win32-common.o" LIBNAME="supafaust-x86.dll" -j4 && \
#make clean && make CXX="x86_64-w64-mingw32-g++" EXTRA_CXXFLAGS="-mtune=atom -fno-PIC -fno-pic -mcmodel=small" LIBS="-static -lwinmm" OBJ_PLATFORM="mednafen/mthreading/MThreading_Win32.o mednafen/win32-common.o" LIBNAME="supafaust-x86_64.dll" -j4 && \
#
