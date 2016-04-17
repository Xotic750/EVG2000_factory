# wps makefile

BUILD_DIR=$(word 1, $(subst /bcmdrivers, /bcmdrivers,$(shell pwd)))
include $(BUILD_DIR)/make.common

CFLAGS += -I.
CFLAGS += -I./common/include
CFLAGS += -I../include
CFLAGS += -I../include/bcmcrypto
CFLAGS += -I../router/bcmupnp/include
CFLAGS += -I../router/bcmupnp/device
CFLAGS += -I../router/bcmupnp/device/WFADevice
CFLAGS += -I../router/eapd
CFLAGS += -I../router/nas
CFLAGS += -I../router/shared
CFLAGS += -I../shared
CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
CFLAGS += -I$(BUILD_DIR)/userspace/private/include
CFLAGS += -I$(BUILD_DIR)/userspace/public/include
CFLAGS += -I$(BUILD_DIR)/userspace/public/include/linux
CFLAGS += -DBCMWPA2
CFLAGS += -DDSLCPE
CFLAGS += -D_REENTRANT 
CFLAGS += -D__linux__
CFLAGS += -DIL_BIGENDIAN
# CFLAGS += -D_TUDEBUGTRACE

LDFLAGS = -Os
LDFLAGS += -L$(EXTRALIBDIR)
LDFLAGS += -lc
LDFLAGS += -lgcc_s
LDFLAGS += -L$(INSTALL_DIR)/lib
LDFLAGS += -lnvram
LDFLAGS += -lwlbcmcrypto
LDFLAGS += -lwlbcmshared
LDFLAGS += -lwlctl
LDFALSG += -ldl
LDFLAGS += -L$(INSTALL_DIR)/lib/public
LDFLAGS += -lcms_msg
LDFLAGS += -L$(INSTALL_DIR)/lib/private
LDFLAGS += $(CMS_COMMON_LIBS)
LDFLAGS += -L.
LDFLAGS += -lwps

-include files

LIBWPS_OBJS         = $(foreach x, $(LIBWPS_FILES), $(shell find . -name "$x" -exec echo $(x:.c=.o) ";"))
WPS_MONITOR_OBJS    = $(foreach x, $(WPS_MONITOR_FILES), $(shell find . -name "$x" -exec echo $(x:.c=.o) ";"))
WPS_AP_OBJS         = $(foreach x, $(WPS_AP_FILES), $(shell find . -name "$x" -exec echo $(x:.c=.o) ";"))
WPS_ENR_OBJS        = $(foreach x, $(WPS_ENR_FILES), $(shell find . -name "$x" -exec echo $(x:.c=.o) ";"))

vpath %.c ./brcm_apps/apps
vpath %.c ./brcm_apps/wl
vpath %.c ./linux/ap
vpath %.c ./common/ap
vpath %.c ./common/enrollee
vpath %.c ./common/registrar
vpath %.c ./common/shared
vpath %.c ./common/sta

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

mkversion:
	sh ./mkversion.sh ./common/include/version.h.in ./common/include/wps_version.h "$(PROD_TAG)"

libwps.so: $(LIBWPS_OBJS)
ifneq ($(strip $(LIBWPS_OBJS)),)
	$(CC) -shared -Os -o $@ $^
endif

wps_monitor.lib: $(WPS_MONITOR_OBJS)
ifneq ($(strip $(WPS_MONITOR_OBJS)),)
	$(AR) -rcs $@ $^
endif

wps_monitor: wps_gpio_dslcpe.o wps_monitor.lib libwps.so
	$(CC) -o $@ $^ $(LDFLAGS)

wps_ap : $(WPS_AP_OBJS) libwps.so
ifneq ($(strip $(WPS_AP_OBJS)),)
	$(CC) -o $@ $(WPS_AP_OBJS) $(LDFLAGS)
endif

wps_enr : $(WPS_ENR_OBJS) libwps.so
ifneq ($(strip $(WPS_ENR_OBJS)),)
	$(CC) -o $@ $(WPS_ENR_OBJS) $(LDFLAGS)
endif

install: dynamic
	install -m 755 libwps.so $(INSTALL_DIR)/lib
	install -m 755 wps_monitor $(INSTALL_DIR)/bin
	install -m 755 wps_ap $(INSTALL_DIR)/bin
	install -m 755 wps_enr $(INSTALL_DIR)/bin

clean:
	rm -f wps_monitor
	rm -f wps_gpio_dslcpe.o
	rm -f $(LIBWPS_OBJS)
	rm -f $(WPS_MONITOR_OBJS)
	rm -f $(WPS_AP_OBJS)
	rm -f $(WPS_ENR_OBJS)
ifneq ($(strip $(LIBWPS_OBJS)),)
	rm -f libwps.so
endif
ifneq ($(strip $(WPS_MONITOR_FILES)),)
	rm -f wps_monitor.lib
endif
ifneq ($(strip $(WPS_AP_FILES)),)
	rm -f wps_ap
endif
ifneq ($(strip $(WPS_ENR_FILES)),)
	rm -f wps_enr
endif

dynamic: libwps.so wps_monitor wps_ap wps_enr
static:
