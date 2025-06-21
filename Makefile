Project := kern

boot   	:= boot
kernel 	:= kernel8
secmon	:= secmon

BUILD 	  := build
RESOURCES := resources
IMAGE_SIZE_MB := 32

export PROJECT_ROOT := $(CURDIR)

CC=clang
OC=llvm-objcopy
LD=ld.lld
INCLUDE := include
LIBS    := $(BUILD)/libraries/libraries.a

COMMON := -Wall -g -O0 -ffreestanding -nostdinc -nostdlib -march=armv8-a -mcpu=cortex-a72+nosimd
CFLAGS := -I$(INCLUDE) $(COMMON)
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__ $(COMMON)

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S_boot := $(call rwildcard, boot, *.S)
SRCS_C_boot := $(call rwildcard, boot, *.c)
SRCS_S_kern := $(call rwildcard, kernel, *.S)
SRCS_C_kern := $(call rwildcard, kernel, *.c)

OBJS_boot := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_boot)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_boot))

OBJS_kern := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_kern)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_kern))

.PHONY: all clean library KIPs secmon kernel boot image qemu qemu-gdb

all: setup library KIPs secmon kernel boot image

clean:
	@rm -fr $(BUILD) $(kernel).elf $(secmon).elf $(boot).elf $(kernel).img $(secmon).bin $(boot).bin $(Project).img
	$(MAKE) -C KIPs clean
	$(MAKE) -C secmon clean
	$(MAKE) -C libraries clean

qemu: $(kernel).elf
	qemu-system-aarch64 -M raspi4b -serial stdio -kernel $(boot).bin -drive file=$(Project).img,if=sd,format=raw

qemu-gdb: $(kernel).elf
	qemu-system-aarch64 -M raspi4b -serial stdio -kernel $(boot).bin -sd $(Project).img -s -S

image:
	dd if=/dev/zero of=$(Project).img bs=1M count=$(IMAGE_SIZE_MB)
	mkfs.fat -F 32 -n "BOOT" $(Project).img
	for file in $(RESOURCES)/bootfs/*; do \
		mcopy -i $(Project).img "$$file" "::/"; \
	done
	mcopy -i $(Project).img "boot.bin" "::/"
	mcopy -i $(Project).img "secmon.bin" "::/"

library:
	$(MAKE) -C libraries
	cp libraries/libraries.a $(BUILD)/libraries

KIPs:
	$(MAKE) -C KIPs

secmon: 
	$(MAKE) -C secmon
	cp secmon/secmon.bin .

boot: $(boot).elf
	$(OC) $^ -O binary $(boot).bin

kernel: $(kernel).elf
	$(OC) $^ -O binary $(kernel).img

$(boot).elf: $(OBJS_boot) $(LIBS)
	$(LD) -m aarch64elf -nostdlib -T boot/linker.ld -o $@ $^

$(kernel).elf: $(OBJS_kern) $(LIBS)
	$(LD) -m aarch64elf -nostdlib -T kernel/linker.ld -o $@ $^

$(BUILD)/%.o: %.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

setup:
	mkdir -p $(BUILD)/kernel/memory
	mkdir -p $(BUILD)/boot
	mkdir -p $(BUILD)/libraries
