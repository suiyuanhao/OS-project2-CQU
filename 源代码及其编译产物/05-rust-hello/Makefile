obj-m += rust_hello.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	RUSTUP_TOOLCHAIN=1.73.0 $(MAKE) -C $(KDIR) M=$(PWD) LLVM=1 modules

clean:
	RUSTUP_TOOLCHAIN=1.73.0 $(MAKE) -C $(KDIR) M=$(PWD) LLVM=1 clean