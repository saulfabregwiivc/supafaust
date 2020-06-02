CXX=g++
CXXFLAGS=-O2 -fpic -fno-stack-protector -fvisibility=hidden -fsigned-char -fwrapv -funroll-loops -std=c++11 -Wall -I./ -Wa,--fatal-warnings $(EXTRA_CXXFLAGS)
CPPFLAGS=-D_GNU_SOURCE=1
LIBS=-lpthread
LIBNAME=supafaust.so
include common.mk

OBJ=libretro.o $(OBJ_MDFN) $(OBJ_SNES)

%.o:	%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

all:	$(OBJ) $(OBJ_SNES)
	$(CXX) $(CXXFLAGS) -s -shared -o $(LIBNAME) $^ $(LIBS)
	@cat supafaust.tar.xz.zip >> $(LIBNAME)
	@zip --adjust-sfx $(LIBNAME)

clean:
	rm -f -- $(OBJ) $(OBJ_SNES) $(LIBNAME)

.PHONY: all clean

