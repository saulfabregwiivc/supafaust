#!/bin/sh

supafaust_version=`date "+%^b%-d-%Y"`
supafaust_txz="supafaust-$supafaust_version-linux-builds.tar.xz"
tar --owner=root --group=root --mtime="`date`" -c ./supafaust*.so ./README -O | xz > "$supafaust_txz"
#./supafaust*.dll ./COPYING
