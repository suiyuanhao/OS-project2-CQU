obj-m += rust_char_device.o
rust_char_device-y := rust_char_device_core.o char_shim.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	RUSTUP_TOOLCHAIN=1.73.0 $(MAKE) -C $(KDIR) M=$(PWD) LLVM=1 modules

clean:
	RUSTUP_TOOLCHAIN=1.73.0 $(MAKE) -C $(KDIR) M=$(PWD) LLVM=1 clean