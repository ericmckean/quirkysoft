Overview
=========

If you have downloaded this in a source tar.gz, then run

    ./configure
    make

If you are using a version from git, then you will need to install waf from
http://waf.googlecode.com/ - at least version 1.4.1, perhaps even the svn trunk
version. Read on for more details...

Prerequisites
=============

First I'll assume you are using Ubuntu 7.10+, or some similar modern-ish Linux
distro. Otherwise you may have to modify these steps a little.

Download and install DevkitARM, libnds, libdswifi and libfat. These can be
found at the website http://sourceforge.net/projects/devkitpro/

An experimental Linux installer is available on the DKP forums here
http://forums.devkitpro.org/viewtopic.php?f=25&t=10 but the tar.gz files from
sourceforge are not too difficult to install.

Set the variable DEVKITPRO to point to the /path/to/devkitpro and DEVKITARM to
point to $DEVKITPRO/devkitARM, something like this:

    export DEVKITPRO=$HOME/devkitpro
    export DEVKITARM=$DEVKITPRO/devkitARM

[ TIP: place these lines in your ~/.bashrc file ]

Now install the latest devkitArm:

    mkdir -p $DEVKITPRO
    cd $DEVKITPRO
    tar xjvf /path/to/devkitARM_r*-linux.tar.bz2

Install libnds and friends, either from the precompiled binaries, or from
source:

    mkdir /tmp/libnds
    cd /tmp/libnds
    tar xjvf /path/to/libnds-src-*.tar.bz2
    make install

Repeat for dswifi and libfat. This ensures your compiler is installed
correctly. Note that the devkitPro tar balls don't have top level directories,
so be careful to extract them into their own new directory or face having
files where you didn't want them.

Additionally and optionally, download the DLDI patch tool and appropriate
driver for your card. See chishm's DLDI website http://chishm.drunkencoders.com/DLDI/

In order to automatically have the build system generate a patched ".nds" file
you must specifiy the path to "your.dldi" driver and have the dlditool in
$DEVKITARM/bin.  This is not required if your card automatically patches
homebrew.

Building dependencies
=====================

The following tools are needed:

    sudo apt-get install patch
    sudo apt-get install subversion

And also python 2.5. To build the dependencies for the DS version. Assuming you
are in the ame directory as this file now:

    here=$PWD
    cd /tmp/
    $here/tools/download-libs.sh

This should install libpng, libgif, libjpeg, zlib, unzip and matrixSSL into
the $DEVKITPRO/libnds directory. 

Alternatively, use the pre-compiled libraries that are available from
http://code.google.com/p/quirkysoft/downloads/list and copy them to
$DEVKITPRO/libnds/ using the install.sh script.

Follow its final instructions to install unzip and matrixSSL for Linux. The
following development libraries are also needed:

    sudo apt-get install libcppunit-dev
    sudo apt-get install libpng-dev
    sudo apt-get install libgif-dev
    sudo apt-get install libjpeg-dev
    sudo apt-get install libgl1-mesa-dev
    sudo apt-get install zlib1g-dev
    sudo apt-get install libsdl1.2-dev
    sudo apt-get install sox

And maybe some more ^_^, patches to the build instructions are welcome.

Almost there!
=============

That's the tough bit out the way, hopefully. Now to compile. You can
use the usual configure style build:

    ./configure
    make

Or if you are familiar wit the waf build system, the equivalent commands are
valid.

If this works, you should now have bunjalloo.nds and bunjalloo executable in
the build directory. First see if the Linux version works:

    cd bunjalloo
    ../_build_/sdl/bunjalloo

If that worked, try out the DS build...

Installing
==========

There build system has a flag '--install-to' that makes the installation of the
required files onto your DLDI supported device easy. In these steps
/media/$disk is the mount point of your media card, you will have to replace it
with the real location:

    ./waf --install-to=/media/$disk install
    dlditool /path/to/your.dldi /media/$disk/bunjalloo.nds

Once you have done that, and assuming you don't alter things in the data
directory, future builds can be updated by simply running:

    cp -v _build_/default/bunjalloo-patched.nds /media/$disk/bunjalloo.nds

See this page too for more details.
http://code.google.com/p/quirkysoft/wiki/Compiling

Good luck, Commander.
--- MESSAGE ENDS
