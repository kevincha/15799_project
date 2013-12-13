KERNELSRCDIR = ../
BUILD_DIR := $(shell pwd)
VERBOSE = 0

#
obj-m := memtest.o

#
memtest-objs := kernel.o

KERNELSRCDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNELSRCDIR) SUBDIRS=$(BUILD_DIR) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_VERBOSE=$(VERBOSE) modules

clean:
	make -C $(KERNELSRCDIR) SUBDIRS=$(BUILD_DIR) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_VERBOSE=$(VERBOSE) clean

