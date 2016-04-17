# eapd makefile

BUILD_DIR=$(word 1, $(subst /bcmdrivers, /bcmdrivers,$(shell pwd)))
include $(BUILD_DIR)/make.common

CFLAGS += -I.
CFLAGS += -I../../include
CFLAGS += -I../../shared
CFLAGS += -I../../router/shared
CFLAGS += -I../../router/nas
CFLAGS += -DBCMWPA2 
CFLAGS += -DEAPD_WKSP_AUTO_CONFIG
CFLAGS += -DIL_BIGENDIAN
CFLAGS += -DDSLCPE
CFLAGS += -Wall

LDFLAGS = -Os 
LDFLAGS += -L$(EXTRALIBDIR)
LDFLAGS += -lc
LDFLAGS += -lgcc_s
LDFLAGS += -L$(INSTALL_DIR)/lib
LDFLAGS += -lnvram
LDFLAGS += -lwlbcmcrypto
LDFLAGS += -lwlbcmshared

-include files

OBJS = $(foreach x, $(FILES), $(shell find . -name "$x" -exec echo $(x:.c=.o) ";"))

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

eapd: $(OBJS)
ifneq ($(strip $(OBJS)),)
	$(CC) -o $@ $^ $(LDFLAGS)
endif

install: eapd
	install -m 755 $< $(INSTALL_DIR)/bin

clean:
	rm -f $(OBJS)
ifneq ($(strip $(OBJS)),)
	rm -f eapd
endif

dynamic static: eapd
