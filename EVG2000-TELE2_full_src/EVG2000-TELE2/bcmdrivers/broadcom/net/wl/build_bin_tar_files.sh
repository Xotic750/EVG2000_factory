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

echo Tar release on $imp

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
  --exclude=*.cmd \
  --exclude=*.contrib* \
  --exclude=*.db \
  --exclude=*.keep* \
  --exclude=*.merge \
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

bin_found=no

for x in `find $imp -maxdepth 1 -name *.o_save`; do
  bin_found=yes
done

if [ $bin_found != yes ]; then
  exit
fi

tar -zcvf wlan-bin-all-$ver.tgz makefile.wlan $imp $exclude

if  [ $imp == "impl6" ]; then
  echo Add Dongle files to $imp release
  gunzip wlan-bin-all-$ver.tgz
  tar --append \
      --file=wlan-bin-all-$ver.tar \
             $imp/usbdev/usbdl/bcmdl \
             $imp/usbdev/libusb-0.1.12/libusb-0.1.12.tgz \
             $imp/dongle/rte/wl/builds/4322-bmac/rom-ag/rtecdc.trx
  gzip wlan-bin-all-$ver.tar
  mv wlan-bin-all-$ver.tar.gz wlan-bin-all-$ver.tgz
fi

#############################################################################
# Put all files in router version directory

if [ ! -d wlan-router.$ROUTER_VERSION_STR/bin ]; then
  mkdir -p wlan-router.$ROUTER_VERSION_STR/bin
fi

mv *tgz wlan-router.$ROUTER_VERSION_STR/bin

# End of file
