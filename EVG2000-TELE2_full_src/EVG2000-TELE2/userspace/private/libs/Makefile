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
# In most cases, you only need to modify this first section.
# Just add libraries to LIB_DIRS
#
#LIB_DIRS = mdm cms_core cms_dal cms_cli atmctl xdslctl bcmssl omci spuctl pwrctl \
#           nanoxml

LIB_DIRS = cms_core cms_dal bcmssl spuctl 

#
# We need to declare these directory names as phony targets,
# otherwise make will think the directories themselves are
# the targets and will never execute the cmds.
# See GNU Make, 4.6 
#
.PHONY : $(LIB_DIRS)
all    : $(LIB_DIRS)


#
# Cleaning rules
# See example in GNU Make, 4.6
#
clean: lib_clean

lib_clean:
	for dir in $(LIB_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ifneq ($(strip $(BUILD_VODSL)),)
include $(BUILD_DIR)/make.voice
endif



##############################################################
# 
# Rules for inidividual libs
# libs can be compiled or not depending on the profile (which
# is modified via make menuconfig)
#
##############################################################

# always build this lib
mdm:
	$(MAKE) -C $@


# always build this lib
cms_core:
	$(MAKE) -C $@

# always build this lib
cms_dal:
	$(MAKE) -C $@

# always build this lib
nanoxml:
	$(MAKE) -C $@


# Always build this lib, but if CLI_CMD and CLI_MENU is not defined
# it will be a very small lib (should be a zero length lib)
cms_cli:
	$(MAKE) -C $@


ifneq ($(strip $(BUILD_XTMCTL)),)
atmctl:
	$(MAKE) -C atmctl $(BUILD_XTMCTL)
else
atmctl:
	@echo "skipping atmctl (not configured)"
endif


ifneq ($(strip $(BUILD_XDSLCTL)),)
xdslctl:
	$(MAKE) -C xdslctl $(BUILD_XDSLCTL)
else
xdslctl:
	@echo "skipping xdslctl (not configured)"
endif


ifneq ($(strip $(BUILD_TR69C_BCM_SSL)),)
bcmssl: sanity_check
	(tar xkjf bcmssl.tar.bz2 2> /dev/null || true)
	$(MAKE) -C bcmssl
else
bcmssl: sanity_check
	@echo "skipping bcmssl (not configured)"
endif

ifneq ($(strip $(BUILD_GPON_OMCI)),)
omci:
	$(MAKE) -C $@
else
omci:
	@echo "skipping omci (not configured)"
endif

ifneq ($(strip $(BUILD_SPUCTL)),)
spuctl:
	$(MAKE) -C spuctl $(BUILD_SPUCTL)
else
spuctl:
	@echo "skipping spuctl (not configured)"
endif

ifneq ($(strip $(BUILD_PWRCTL)),)
pwrctl:
	$(MAKE) -C pwrctl $(BUILD_PWRCTL)
else
pwrctl:
	@echo "skipping pwrctl (not configured)"
endif
