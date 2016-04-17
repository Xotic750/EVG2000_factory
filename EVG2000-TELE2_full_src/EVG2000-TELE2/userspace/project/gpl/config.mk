#
# Copyright 2005  Hon Hai Precision Ind. Co. Ltd.
#  All Rights Reserved.
# No porti

#For Compiler Option, add by Jasmine Yang, 03/27/2006

#water, 05/22/2008, @samba
SAMBA_ENABLE_FLAG=y
CONFIG_NTFS_3G=y
CONFIG_MTOOLS=y
FTP_ACCESS_USB_FLAG=y
PLATFORM_TYPE=EVG2000

ifeq ($(FTP_ACCESS_USB_FLAG),y)
CFLAGS += -DFTP_ACCESS_USB
endif

#jenny, 08/20/2008, @automount
AUTOMOUNT_FLAG=y

ifeq ($(FW_TYPE),TI)
PPPOE_RELAY_FLAG=y
endif

ifeq ($(FW_TYPE),SINGTEL)
CFLAGS	+= -DSingTel
endif

ifeq ($(FW_TYPE),TELE2)
CFLAGS += -DSUPPORT_TR111
CFLAGS += -DU12H154_TELE2 
endif

CFLAGS	+= -DDNSMASQ_FOR_MULTIPLE_LAN_WAN -DBSP_4_X_X -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx

export CFLAGS
