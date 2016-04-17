#***********************************************************************
#
#  Copyright (c) 2006  Broadcom Corporation
#  All Rights Reserved
#
# 
# 
# This program is the proprietary software of Broadcom Corporation and/or its 
# licensors, and may only be used, duplicated, modified or distributed pursuant 
# to the terms and conditions of a separate, written license agreement executed 
# between you and Broadcom (an "Authorized License").  Except as set forth in 
# an Authorized License, Broadcom grants no license (express or implied), right 
# to use, or waiver of any kind with respect to the Software, and Broadcom 
# expressly reserves all rights in and to the Software and all intellectual 
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE 
# NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY 
# BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE. 
# 
# Except as expressly set forth in the Authorized License, 
# 
# 1. This program, including its structure, sequence and organization, 
#    constitutes the valuable trade secrets of Broadcom, and you shall use 
#    all reasonable efforts to protect the confidentiality thereof, and to 
#    use this information only in connection with your use of Broadcom 
#    integrated circuit products.  (Note this clause prohibits you from 
#    linking, either statically or dynamically, this program with any software 
#    that is licensed under the GPL, as the terms of the GPL would force you 
#    to release the source code of this program, thus violating the 
#    confidentiality aspect of this clause.) 
# 
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH 
#    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
#    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, 
#    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
#    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
#    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR 
#    PERFORMANCE OF THE SOFTWARE. 
# 
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR 
#    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, 
#    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
#    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN 
#    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
#    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE 
#    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS 
#    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY 
#    LIMITED REMEDY. 
#
#
#***********************************************************************/


#
# To add an app, add it to APP_DIRS, and write the target rule in the
# last section of this Makefile
#
#APP_DIRS = smd ssk omcid laserd tr69c wlan httpd consoled telnetd \
#           xdslctl atmctl ethctl cmfctl cmfviz \
#           ddnsd dnsprobe dnsspoof igmp sntp upnp ippd dsldiagd\
#           snmp tr64 mld hotplug cvtpsi spuctl pwrctl

APP_DIRS = wlan telnetd \
           ethctl cmfctl cmfviz \
           ippd \
           mld hotplug cvtpsi spuctl


#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#
.PHONY : $(APP_DIRS)
all : $(APP_DIRS)


#
# Cleaning rules
# See example in GNU Make, 4.6
#
clean: 
	for dir in $(APP_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ifneq ($(strip $(BUILD_VODSL)),)
include $(BUILD_DIR)/make.voice
endif

export BUILD_DIR



##############################################################
# 
# Rules for inidividual apps
# apps can be compiled or not depending on the profile (which
# is modified via make menuconfig)
#
##############################################################

# Always build system management daemon (smd), this is the
# daemon that replaces cfm
smd: 
	$(MAKE) -C $@

# Always build SMD side-kick (ssk), this is kind of
# a helper daemon for smd.
ssk:
	$(MAKE) -C $@


#
# There are three possible variables for tr69c in make menuconfig:
# BUILD_TR69C 
# BUILD_TR69C_SSL
# BUILD_TR69C_BCM_SSL
# (or none)
# If any of the tr69c variants are defined, just go into the tr69c directory
# and make the target.
#
ifneq ($(strip $(BUILD_TR69C)),)
tr69c:
	$(MAKE) -C $@
else
ifneq ($(strip $(BUILD_TR69C_SSL)),)
tr69c:
	$(MAKE) -C $@
else
ifneq ($(strip $(BUILD_TR69C_BCM_SSL)),)
tr69c:
	$(MAKE) -C $@
else
tr69c:
	@echo "skipping tr69c (not configured)"
endif
endif
endif


# omcid
ifneq ($(strip $(BUILD_GPON_OMCI)),)
omcid: 
	$(MAKE) -C $@
laserd: 
	$(MAKE) -C $@
else
omcid:
	@echo "skipping omcid (not configured)"
laserd:
	@echo "skipping laserd (not configured)"
endif


# httpd
ifneq ($(strip $(BUILD_HTTPD)),)
httpd: 
	$(MAKE) -C $@
else
ifneq ($(strip $(BUILD_HTTPD_SSL)),)
httpd: 
	$(MAKE) -C $@
else
httpd:
	@echo "skipping httpd (not configured)"
endif
endif


# consoled
ifneq ($(strip $(BUILD_CONSOLED)),)
consoled:
	$(MAKE) -C $@
else
consoled:
	@echo "skipping consoled (not configured)"
endif


# telnetd
ifneq ($(strip $(BUILD_TELNETD)),)
telnetd:
	$(MAKE) -C $@
else
telnetd:
	@echo "skipping telnetd (not configured)"
endif


# atmctl
ifneq ($(strip $(BUILD_XTMCTL)),)
atmctl:
	$(MAKE) -C $@
else
atmctl:
	@echo "skipping atmctl (not configured)"
endif


# xdslctl
ifneq ($(strip $(BUILD_XDSLCTL)),)
xdslctl: 
	$(MAKE) -C $@
else
xdslctl:
	@echo "skipping xdslctl (not configured)"
endif


#dynamic dns
ifneq ($(strip $(BUILD_DDNSD)),)
ddnsd: sanity_check
	$(MAKE) -C ddnsd $(BUILD_DDNSD)
else
ddnsd: sanity_check
	@echo "skipping ddnsd (not configured)"
endif



# dnsprobe
ifneq ($(strip $(BUILD_DNSPROBE)),)
dnsprobe: sanity_check
	$(MAKE) -C dnsprobe $(BUILD_DNSPROBE)
else
dnsprobe: sanity_check
	@echo "skipping dnsprobe (not configured)"
endif


# dnsspoof
ifneq ($(strip $(BUILD_DNSSPOOF)),)
dnsspoof: sanity_check
	$(MAKE) -C dnsspoof $(BUILD_DNSSPOOF)
else
dnsspoof: sanity_check
	@echo "skipping dnsspoof (not configured)"
endif


#Always compile Ethernet control utility
ethctl: sanity_check
	$(MAKE) -C ethctl dynamic


ifneq ($(strip $(BUILD_IGMP)),)
igmp: sanity_check
	$(MAKE) -C igmp $(BUILD_IGMP)
else
igmp: sanity_check
	@echo "skipping igmp (not configured)"
endif


# Time synchronization daemon
ifneq ($(strip $(BUILD_SNTP)),)
sntp: sanity_check
	$(MAKE) -C sntp $(BUILD_SNTP)
else
sntp: sanity_check
	@echo "skipping sntp (not configured)"
endif


# UPNP is dependent on iptables
ifneq ($(strip $(BUILD_IPTABLES)),)
ifneq ($(strip $(BUILD_UPNP)),)
upnp: sanity_check
	$(MAKE) -C upnp $(BUILD_UPNP)
else
upnp: sanity_check
	@echo "skipping upnp (not configured)"
endif
else
upnp: sanity_check
	@echo Warning: upnp requires iptables, please enable iptables.
endif


# IP Printing Daemon
ifneq ($(strip $(BUILD_IPPD)),)
ippd: sanity_check
	$(MAKE) -C ippd $(BUILD_IPPD)
else
ippd: sanity_check
	@echo "skipping ippd (not configured)"
endif


# DSL Diag Daemon
ifneq ($(strip $(BUILD_DSLDIAGD)),)
dsldiagd: sanity_check
	$(MAKE) -C $@
else
dsldiagd: sanity_check
	@echo "skipping dsldiagd (not configured)"
endif


# Packet CMF Control Utility
ifneq ($(strip $(BUILD_CMFCTL)),)
cmfctl: sanity_check
	$(MAKE) -C $@ $(BUILD_CMFCTL)
else
cmfctl: sanity_check
	@echo "Skipping cmfctl application (Packet CMF not configured)"
endif

# Packet CMF Viz Application
ifneq ($(strip $(BUILD_CMFVIZ)),)
cmfviz: sanity_check
	$(MAKE) -C cmfviz $(BUILD_CMFVIZ)
else
cmfviz: sanity_check
	@echo "Skipping cmfviz application (Packet CMF not configured)"
endif

# wlan software
ifneq ($(strip $(BUILD_WLCTL)), )
wlan: sanity_check
	$(MAKE) -C $@
else
wlan:
endif

# snmp
ifneq ($(strip $(BUILD_SNMP)),)
snmp: sanity_check
#	$(MAKE) -C snmp $(BUILD_SNMP)
	$(MAKE) -C $@
else
snmp: sanity_check
	@echo "skipping snmp (not configured)"
endif

# tr64
ifneq ($(strip $(BUILD_TR64)),)
tr64: sanity_check
	$(MAKE) -C tr64
else
tr64: sanity_check
	@echo "skipping tr64 (not configured)"
endif

# MLD
ifneq ($(strip $(BUILD_MLD)),)
mld: sanity_check
	$(MAKE) -C mld $(BUILD_MLD)
else
mld: sanity_check
	@echo "skipping mld (not configured)"
endif

#hotplug software
ifneq ($(strip $(BUILD_NAS)),)
hotplug: sanity_check
	$(MAKE) -C $@ dynamic
else
ifneq ($(strip $(LINUX_KERNEL_USBMASS)),)
hotplug: sanity_check
	$(MAKE) -C $@ dynamic
else
hotplug: sanity_check
	@echo "skipping hotplug (not configured)"
endif
endif

# PSI config file to MDM config file converter program
ifneq ($(strip $(CONVERT_PSI_TO_MDM)),)
cvtpsi: sanity_check
	$(MAKE) -C cvtpsi
else
cvtpsi: sanity_check
	@echo "skipping cvtpsi (not configured)"
endif

# spuctl
ifneq ($(strip $(BUILD_SPUCTL)),)
spuctl:
	$(MAKE) -C $@
else
spuctl:
	@echo "skipping spuctl (not configured)"
endif

# pwrctl
ifneq ($(strip $(BUILD_PWRCTL)),)
pwrctl:
	$(MAKE) -C $@
else
pwrctl:
	@echo "skipping pwrctl (not configured)"
endif
