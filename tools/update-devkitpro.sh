#!/bin/sh

g() {
    python <<EOF
import ConfigParser
c = ConfigParser.ConfigParser()
c.readfp(open("$DEVKITPRO/" + 'devkitProUpdate.ini'))
print c.get("$1", "$2")
EOF
}

. $(dirname $0)/shell_functions.sh

check_devkitpro

cd $DEVKITPRO
here=$(pwd)
url="http://devkitpro.sourceforge.net/devkitProUpdate.ini"
if [ ! -e devkitProUpdate.ini ] ; then
    wget $url || die "Unable to download devkitProUpdate.ini"
fi

section=devkitProUpdate
url=$(g $section URL)
platform=$(uname -m)-linux.tar.bz2

for item in devkitARM libnds libndsfat maxmodds dswifi filesystem defaultarm7 ndsexamples
do
    dl=$(g $item File | sed "s/win32\.exe/$platform/g")
    v=$(g $item Version)
    if [ -e $dl ] ; then
        printf "Looks like you already have $item $v. Download and install anyway? [y/N]"
        read doit
        if [ "x$doit" != "xy" ] ; then
            continue
        fi
    fi
    wget $url/$dl

    case $item in
        devkitARM)
            touch $item-$v
            tar xf $dl
            ;;
        ndsexamples)
            mkdir -p $item-$v
            cd $item-$v
            tar xf ../$dl
            cd ..
            ;;
        *)
            touch $item-$v
            mkdir -p libnds
            cd libnds
            tar xf ../$dl
            cd ..
            ;;
    esac
done
