# SPDX-License-Identifier: GPL-2.0
# Onsemi AF0130 sensor driver for NXP VVCAM framework


# Build a module named af0130.ko
obj-m := af0130.o


#$(info ----------------------------)
#$(info COMPILING module in folder $(CURDIR))
#$(info src_tree=$(srctree))
#$(info obj-m=$(obj-m))
#$(info CC=$(CC))
#$(info CFLAGS=$(CFLAGS))
#$(info EXTRA_FLAGS=$(EXTRA_CFLAGS))
#$(info ----------------------------)


SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) INSTALL_MOD_PATH=${D} modules_install

clean:
	@echo "Nothing to clean"
