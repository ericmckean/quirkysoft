#!/bin/sh

if test -z "$DEVKITPRO" ; then
  echo "No DEVKITPRO variable set. Set the path to devkit pro."
  exit 1
fi

# copy the files
filenames="
lib/libgif.a
lib/libjpeg.a
lib/libmatrixsslstatic.a
lib/libpng.a
lib/libunzip.a
lib/libz.a
include/gif_lib.h
include/ioapi.h
include/jconfig.h
include/jerror.h
include/jmorecfg.h
include/jpeglib.h
include/matrixCommon.h
include/matrixConfig.h
include/matrixSsl.h
include/pngconf.h
include/png.h
include/unzip.h
include/zconf.h
include/zlib.h
"

mkdir .create_deps
cd .create_deps
mkdir -p lib include
for i in $filenames
do
  cp -v $DEVKITPRO/libnds/$i $i
done
cat > install.sh << EOF
#!/bin/sh

if test -z "\$DEVKITPRO" ; then
  echo "No DEVKITPRO variable set. Set the path to devkit pro."
  exit 1
fi
die() {
  echo \$@ 2>&1
  exit 1
}
for i in lib include
do
  dir=\$DEVKITPRO/libnds/\$i
  mkdir -p \$dir || die "Error creating directory \$dir"
  for file in \$i/*
  do
    cp -v \$file \$DEVKITPRO/libnds/\$i
  done
done
EOF

chmod +x install.sh
tar czvf ../bunjalloo-deps-0.6.1.tar.gz install.sh lib include
cd ..
rm -rf .create_deps
