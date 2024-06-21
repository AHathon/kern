kernel := kernel8

BUILD := build

CC=aarch64-linux-gnu-gcc
OC=aarch64-linux-gnu-objcopy
INCLUDE := include
linker_script := linker.ld

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S := $(call rwildcard, kernel, *.s) \
		  $(call rwildcard, libraries, *.s)

SRCS_C := $(call rwildcard, kernel, *.c) \
		  $(call rwildcard, libraries, *.c)

OBJS := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C)) \
		$(patsubst %.s, $(BUILD)/%.o, $(SRCS_S))

.PHONY: all clean kernel qemu qemu-gdb

all: setup $(kernel).elf packKips

clean:
	@rm -fr $(BUILD) $(kernel).elf
	$(MAKE) -C KIPs clean

qemu: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(kernel).elf

$(kernel).elf:  $(OBJS)
	$(CC) -T linker.ld -ffreestanding -O2 -v -nostdlib -lgcc -o $@ $^

packKips: $(CURDIR)/KIPs/kip_blob
	$(OC) --update-section .kips=$^ $(kernel).elf

$(CURDIR)/KIPs/kip_blob:
	$(MAKE) -C KIPs

$(BUILD)/%.o: %.s
	$(CC) -I$(INCLUDE) -D__ASSEMBLY__ -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) -I$(INCLUDE) -ffreestanding -O2 -Wall -Wextra -c -o $@ $^

setup:
	mkdir -p $(BUILD)/kernel/memory
	mkdir -p $(BUILD)/libraries