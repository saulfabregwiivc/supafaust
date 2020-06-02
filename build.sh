#!/bin/sh
git -C . archive --format=tar --prefix=supafaust/ HEAD^{tree} | xz > "supafaust.tar.xz" && \
zip supafaust.tar.xz.zip supafaust.tar.xz && \
./build-arm.sh && \
./build-x86.sh && \
true
