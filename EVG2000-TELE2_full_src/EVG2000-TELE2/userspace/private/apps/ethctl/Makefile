dynamic: all generic_exe_install

clean:
	rm -f ethctl


all:
	cp ethctl_$(PROFILE)_save ethctl

CURR_DIR := $(shell pwd)
BUILD_DIR := $(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR := $(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common
EXE = ethctl
