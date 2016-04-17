#!/bin/bash

#############################################################################
# Find the impl from user input.

if [ -z $1 ]; then
  echo "Which impl to build?  Example:" $0 "7"
  exit
else
  imp="impl"$1
fi

if ! [ -d $impl ]; then
  exit
fi

echo Tar Release On $imp

#############################################################################
# Add DSLCPE version to EPI_VERSION_STR in include/epivers.h.
# This is necessary for that makefile.wlan may have not been called.

. ../../../../version.make
. ./$imp/dslcpe_wlan_minor_version

DSLCPE_WLAN_VERSION="\
    cpe$BRCM_VERSION.$BRCM_VERSION$BRCM_RELEASE.$DSLCPE_WLAN_MINOR_VERSION"

sed -ne "s/DSLCPE_WLAN_VERSION/$DSLCPE_WLAN_VERSION/" \
    -ne "s/.*EPI_VERSION_STR.*\"\(.*\)\".*\(cpe.*\)/EPI_VERSION_STR=\1\2/p" \
    < $imp/include/epivers.h > $imp/epivers

dos2unix $imp/epivers
. $imp/epivers
ver=$EPI_VERSION_STR

sed -ne "s/.*ROUTER_VERSION_STR.*\"\(.*\)\"/ROUTER_VERSION_STR=\1/p" \
    < `find $imp -name router_version.h` > $imp/rver

dos2unix $imp/rver
. $imp/rver
rver=$ROUTER_VERSION_STR

#############################################################################
# These files are to be excluded.

exclude=" \
  --wildcards \
  --wildcards-match-slash \
  --exclude=.*.db \
  --exclude=*.a \
  --exclude=*.cmd \
  --exclude=*.contrib* \
  --exclude=*.db \
  --exclude=*.keep* \
  --exclude=*.lib \
  --exclude=*.merge \
  --exclude=*.o \
  --exclude=*.o_save \
  --exclude=*.obj \
  --exclude=*.save \
  --exclude=*.so \
  --exclude=*.tmp \
  --exclude=*build \
  --exclude=tags \
  --exclude=dongle \
  --exclude=$imp/shared/cfe \
  --exclude=$imp/shared/linux \
  --exclude=$imp/shared/nvram \
  --exclude=$imp/shared/zlib \
  --exclude=$imp/usbdev \
"

#############################################################################
# Some applications need other modules' header files

h_share=" \
  $imp/include/bcmparams.h \
  $imp/include/bcmwpa.h \
  $imp/include/UdpLib.h \
  `find $imp/router/shared -name *.h` \
"

h_crypt=`find $imp/include/bcmcrypto -name *.h`
h_proto=`find $imp/include/proto -name *.h`
h_upnp=`find $imp/router/bcmupnp -name *.h`
h_eapd=`find $imp/router/eapd -name *.h`
h_nas=`find $imp/router/nas -name *.h`
h_wps=`find $imp/wps/common/include -name *.h`

#############################################################################
# !!!!!!!!!!! Do not use wildcard directly with tar !!!!!!!!!!!!!!!!!!!!!!!!!

tar -zcvf wlan-all-src-$ver.tgz makefile.wlan $imp $exclude

if  [ $imp == "impl6" ]; then
  echo Add Dongle Files to wlan-all-src-$ver.tgz
  gunzip wlan-all-src-$ver.tgz
  tar --append \
      --file=wlan-all-src-$ver.tar \
             $imp/makefile.wlan \
             $imp/usbdev/usbdl/bcmdl \
             $imp/usbdev/libusb-0.1.12/libusb-0.1.12.tgz \
             $imp/dongle/rte/wl/builds/4322-bmac/rom-ag/rtecdc.trx
  gzip wlan-all-src-$ver.tar
  mv wlan-all-src-$ver.tar.gz wlan-all-src-$ver.tgz
fi

if [ -d $imp/router ]; then
  tar -zcvf upnp-src-$ver.tgz \
    $imp/router/bcmupnp \
    $h_share \
    $h_crypt \
    $h_wps \
    $exclude
  tar -zcvf eapd-src-$ver.tgz \
    $imp/router/eapd \
    $h_share \
    $h_proto \
    $h_nas \
    $exclude
  tar -zcvf lltd-src-$ver.tgz \
    $imp/router/lltd \
    $h_share \
    $exclude
  tar -zcvf nas-src-$ver.tgz \
    $imp/router/nas \
    $h_share \
    $h_crypt \
    $h_proto \
    $h_eapd \
    $exclude
  tar -zcvf wps-src-$ver.tgz \
    $imp/wps \
    $h_share \
    $h_crypt \
    $h_proto \
    $h_eapd \
    $h_upnp \
    $exclude
  tar -zcvf wl-src-$ver.tgz \
    makefile.wlan \
    $imp/dslcpe_wlan_minor_version \
    $imp/Makefile \
    $imp/bcmcrypto \
    $imp/emf/emf \
    $imp/emf/igs \
    $imp/include \
    $imp/router/emf/emf \
    $imp/router/emf/igs \
    $imp/router/shared \
    $imp/shared/ \
    $imp/wl \
    $exclude
else
  tar -zcvf lltd-src-$ver.tgz $imp/lltd $exclude
  tar -zcvf nas-src-$ver.tgz  $imp/nas $exclude
  tar -zcvf wsc-src-$ver.tgz  $imp/wsc $exclude
  tar -zcvf wl-src-$ver.tgz \
    makefile.wlan \
    $imp/dslcpe_wlan_minor_version \
    $imp/Makefile \
    $imp/bcmcrypto \
    $imp/include \
    $imp/shared/ \
    $imp/wl \
    $exclude
fi

if  [ $imp == "impl6" ]; then
  echo Add Dongle Files to wl-src-$ver.tgz
  gunzip wl-src-$ver.tgz
  tar --append \
      --file=wl-src-$ver.tar \
             $imp/makefile.wlan \
             $imp/usbdev/usbdl/bcmdl \
             $imp/usbdev/libusb-0.1.12/libusb-0.1.12.tgz \
             $imp/dongle/rte/wl/builds/4322-bmac/rom-ag/rtecdc.trx
  gzip wl-src-$ver.tar
  mv wl-src-$ver.tar.gz wl-src-$ver.tgz
fi

#############################################################################
# Put all files in router version directory

if [ ! -d wlan-router.$ROUTER_VERSION_STR/src ]; then
  mkdir -p wlan-router.$ROUTER_VERSION_STR/src
fi

mv *tgz wlan-router.$ROUTER_VERSION_STR/src

# End of file
