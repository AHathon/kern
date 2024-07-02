kernel := kernel8

BUILD := build

CC=clang
OC=llvm-objcopy
LD=ld.lld
INCLUDE := include

CFLAGS := -I$(INCLUDE) -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S := $(call rwildcard, kernel, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C := $(call rwildcard, kernel, *.c) \
		  $(call rwildcard, libraries, *.c)

OBJS := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S))

.PHONY: all clean kernel qemu

all: setup kernel packKips

clean:
	@rm -fr $(BUILD) $(kernel).elf
	$(MAKE) -C KIPs clean

qemu: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(kernel).elf

kernel: $(kernel).elf
	$(OC) $^ -O binary $(kernel).img

$(kernel).elf:  $(OBJS)
	$(LD) -m aarch64elf -nostdlib -T linker.ld -o $@ $^

packKips: $(CURDIR)/KIPs/kip_blob
	$(OC) --update-section .kips=$^ $(kernel).elf

$(CURDIR)/KIPs/kip_blob:
	$(MAKE) -C KIPs

$(BUILD)/%.o: %.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

setup:
	mkdir -p $(BUILD)/kernel/memory
	mkdir -p $(BUILD)/libraries