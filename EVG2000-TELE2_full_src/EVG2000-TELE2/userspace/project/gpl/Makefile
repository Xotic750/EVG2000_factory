#
# Copyright 2005  Hon Hai Precision Ind. Co. Ltd.
#  All Rights Reserved.
# No portions of this material shall be reproduced in any form without the
# written permission of Hon Hai Precision Ind. Co. Ltd.
#
# All information contained in this document is Hon Hai Precision Ind.
# Co. Ltd. company private, proprietary, and trade secret property and
# are protected by international intellectual property laws and treaties.
#
# $Id$
#

include config.in
include config.mk


#
# Paths
#

SUBDIRS := 

SUBDIRS += iproute2-2.6.25
#water, 06/25/2008, port udhcpd from dg834guv5
SUBDIRS += udhcpd
SUBDIRS += dnsmasq-2.15
SUBDIRS += zebra

# Fiji removed by Jenny Zhao, 05/21/2009 @USB NEW SPEC 
#ifneq ($(AUTOMOUNT_FLAG),y)
#SUBDIRS += automount
#endif

#water, 05/22/2008, @samba
ifeq ($(SAMBA_ENABLE_FLAG),y)
#SUBDIRS	+= samba-3.3.4
SUBDIRS	+= samba-3.0.13-new
SUBDIRS	+= bftpd-1.6.6-new
endif

# for NTFS-3g
ifeq ($(CONFIG_NTFS_3G),y)
SUBDIRS += ntfs-3g-2009.3.8
endif

# for mtools (to read FAT volume label)
ifeq ($(CONFIG_MTOOLS),y)
SUBDIRS += mtools-4.0.10
endif

#water, 08/18/2008, @pppoe-relay for Telecom Italia
ifeq ($(PPPOE_RELAY_FLAG),y)
SUBDIRS += rp-pppoe-3.5/src
endif
all:
	for i in ${SUBDIRS}; do \
		(cd $$i; make) || exit 1; \
	done
	

install:
	for i in ${SUBDIRS}; do \
		(cd $$i; make install) || exit 1; \
	done
	
clean:
	for i in ${SUBDIRS}; do \
		(cd $$i; make clean); \
	done

.PHONY: $(SUBDIRS)
