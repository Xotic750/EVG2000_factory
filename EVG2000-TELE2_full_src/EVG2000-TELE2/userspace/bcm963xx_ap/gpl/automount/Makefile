
include ../config.in
include ../config.mk
HOST_INCLUDE_DIR = $(KERNEL_DIR)/include
#INCLUDE=-I. -I${GCC_CROSS_COMPILER}/include -I${HOST_INCLUDE_DIR}
#$(INCLUDE) -include  $(ODY_AUTOHEADER) ${THUMB_INCLUDE}
#CROSS=${GCC_CROSS_COMPILER}/bin/arm-linux-
#LIBPATH=-L./ -L${GCC_CROSS_COMPILER}/lib
CFLAGS+= -msoft-float -I${HOST_INCLUDE_DIR} \
         -I$(BUILD_DIR)/shared/opensource/include/bcm963xx
#CC=$(CROSS)gcc
#AR=$(CROSS)ar
#ARFLAGS=cr
#LD=$(CROSS)ld
#LDFLAGS= -z defs -z relro
#STRIP=$(CROSS)strip

OBJS=automount.h

all: automount 
	@$(STRIP) -x -R .note -R .comment --strip-unneeded automount 
	cp automount $(INSTALL_DIR)/bin
	@rm -rf *.o *.a

.PHONY: automount 
#$(LIB_PATH)
automount: automount.c $(OBJS) 
	$(CC) -o $@ $^ $(CFLAGS) 
#	$(CC) -o $@ $^ $(CFLAGS) $(LIBPATH)
#$(CC) -o $@ $@.c -lflash $(CFLAGS) $(LIBPATH)

install:	
	cp automount $(INSTALL_DIR)/bin

clean:
	@rm -rf *.o *.a *.so automount 
	@rm -rf  $(INSTALL_DIR)/bin/automount
	@rm -f $(MKLIB_BIN)/automount

odyinstall: all
	cp automount $(MKLIB_BIN)
odyuninstall:
	rm -f $(INSTALL_DIR)/bin/automount
	rm -f $(MKLIB_BIN)/automount
