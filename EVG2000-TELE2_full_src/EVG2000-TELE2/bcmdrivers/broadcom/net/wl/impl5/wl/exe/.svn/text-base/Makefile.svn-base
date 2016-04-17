#
# Makefile for wlctl utility.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /bcmdriver, /bcmdriver,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

PROGRAM_NAME:= wlctl

CFLAGS =-s -Os -fomit-frame-pointer -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
LDFLAGS=-Os -Wl,-allow-shlib-undefined
CFLAGS_EXTRA =

WIRELESS_DRIVER_PATH=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)
INC_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/include
SHARED_WIRELESS_DRIVER_PATH=$(WIRELESS_DRIVER_PATH)/shared

INC_KERNEL_INC_PATH=$(KERNEL_DIR)/include
CFLAGS += -Wall -I$(INC_WIRELESS_DRIVER_PATH) -I$(INC_WIRELESS_DRIVER_PATH)/proto -I$(INC_KERNEL_INC_PATH) -DDSLCPE

ifeq ($(strip $(BUILD_WLCTL)), static)
CFLAGS += -DBUILD_STATIC 
endif
CFLAGS += -DBCMWPA2 -DIL_BIGENDIAN

ifeq ($(DSLCPE_DT_BUILD),1)
CFLAGS	+= -DRADIUS_RESTRICTION
endif

ifeq ($(strip $(BRCM_CHIP)),6338)
CFLAGS += -DDSLCPE_SDIO
endif

ifeq ($(strip $(BUILD_WLCTL_SHLIB)),1) 
CFLAGS += -DDSLCPE_SHLIB
CFLAGS_EXTRA = -fpic
endif

ifneq ($(strip $(RUN_NOISE)),0)
   CFLAGS += -DDSLCPE_VERBOSE
endif

CFLAGS += -DDSLCPE -DDSLCPE_SCBLIST

WLCTLOBJS = wlu.o wlu_cmd.o wlu_iov.o wlu_linux.o
SHAREDOBJS = bcmutils.o bcmwifi.o
MAIN = wlu_linux_dslcpe

ifeq ($(strip $(BUILD_WLCTL_SHLIB)),1) 
WLCTLOBJS += wlctl_cmd_dslcpe.o
endif

MAINOBJ = $(MAIN).obj
OBJS = $(WLCTLOBJS) $(SHAREDOBJS)

ifneq ($(wildcard $(SHARED_WIRELESS_DRIVER_PATH)),)
vpath %.c $(SHARED_WIRELESS_DRIVER_PATH) $(CRYPTO_WIRELESS_DRIVER_PATH)
else
OBJS		:= $(foreach obj,$(OBJS),$(SHARED_WIRELESS_DRIVER_PATH)/$(OBJS)),$(CRYPTO_WIRELESS_DRIVER_PATH)/$(OBJS))
endif

all: lib$(PROGRAM_NAME) $(PROGRAM_NAME)

install:
	install -m 755 $(PROGRAM_NAME) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(PROGRAM_NAME)
	if [ -e lib$(PROGRAM_NAME).so ]; then cp -av lib$(PROGRAM_NAME).so $(INSTALL_DIR)/lib; fi
	mkdir -p $(INSTALL_DIR)/sbin
	ln -sf $(PROGRAM_NAME) $(INSTALL_DIR)/bin/wl

dynamic: all install

.c.o:
	$(CC) -DPROTOTYPES $(CFLAGS) $(CFLAGS_EXTRA) -c $<

$(MAINOBJ): $(MAIN).c
	$(CC) $(CFLAGS) -o $@ -c $<

ifeq ($(strip $(BUILD_WLCTL_SHLIB)),1) 

lib$(PROGRAM_NAME): $(OBJS) 
	$(LD) -shared -soname lib$(PROGRAM_NAME).so -o lib$(PROGRAM_NAME).so $(OBJS) -L$(LIBDIR) -lc

$(PROGRAM_NAME): lib$(PROGRAM_NAME) $(MAINOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -L. -o $(PROGRAM_NAME) $(MAINOBJ) -l$(PROGRAM_NAME)

else

$(PROGRAM_NAME): $(MAINOBJ) $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM_NAME) $(MAINOBJ) $(OBJS)

lib$(PROGRAM_NAME):

endif



clean:
	-rm -f $(PROGRAM_NAME) *.o  $(PROGRAM_NAME).a lib$(PROGRAM_NAME).so $(MAINOBJ)

