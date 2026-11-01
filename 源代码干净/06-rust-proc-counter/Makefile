obj-m += rust_proc_counter.o

rust_proc_counter-y := rust_proc_counter_core.o proc_shim.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	RUSTUP_TOOLCHAIN=1.73.0 $(MAKE) -C $(KDIR) M=$(PWD) LLVM=1 modules

clean:
	RUSTUP_TOOLCHAIN=1.73.0 $(MAKE) -C $(KDIR) M=$(PWD) LLVM=1 clean