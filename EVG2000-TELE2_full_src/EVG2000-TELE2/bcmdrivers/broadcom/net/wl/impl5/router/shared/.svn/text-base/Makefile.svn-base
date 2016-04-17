# wlbcmshared shared library makefile

BUILD_DIR=$(word 1, $(subst /bcmdrivers, /bcmdrivers,$(shell pwd)))
include $(BUILD_DIR)/make.common

CFLAGS += -I.
CFLAGS += -I../../include
CFLAGS += -I../../shared
CFLAGS += -DBCMSUP_PSK
CFLAGS += -DBCMSUPPL
CFLAGS += -DBCMWPS
CFLAGS += -DBCMWPA2
CFLAGS += -DIL_BIGENDIAN
CFLAGS += -DDSLCPE
CFLAGS += -s
CFLAGS += -Os
CFLAGS += -Wall
CFLAGS += -fomit-frame-pointer

LDFLAGS = -shared
LDFLAGS += -Os

-include files

OBJS = $(foreach x, $(FILES), $(shell find . ../../shared -name "$x" -exec echo $(x:.c=.o) ";"))

vpath %.c ./
vpath %.c ../../shared

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

dynamic: $(OBJS)
ifneq ($(strip $(OBJS)),)
	$(CC) $(LDFLAGS) -o libwlbcmshared.so $^
endif

static: $(OBJS)
ifneq ($(strip $(OBJS)),)
	$(AR) -rcs libwlbcmshared.a $^
endif

install: dynamic
	install -m 755 libwlbcmshared.so $(INSTALL_DIR)/lib

clean:
	rm -f $(OBJS)
ifneq ($(strip $(OBJS)),)
	rm -f libwlbcmshared.a
	rm -f libwlbcmshared.so
endif
