kernel := kernel8
secmon := secmon

BUILD := build

CC=clang
OC=llvm-objcopy
LD=ld.lld
INCLUDE := include

CFLAGS := -I$(INCLUDE) -Wall -O0 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S_kern := $(call rwildcard, kernel, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C_kern := $(call rwildcard, kernel, *.c) \
		  $(call rwildcard, libraries, *.c)

SRCS_S_sec := $(call rwildcard, secmon, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C_sec := $(call rwildcard, secmon, *.c) \
		  $(call rwildcard, libraries, *.c)

OBJS_kern := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_kern)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_kern))
	
OBJS_sec := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_sec)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_sec))

.PHONY: all clean kernel qemu qemu-gdb

all: setup secmon kernel packKips

clean:
	@rm -fr $(BUILD) $(kernel).elf $(secmon).elf
	$(MAKE) -C KIPs clean

qemu: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(kernel).img

qemu-gdb: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(kernel).img -s -S

secmon: $(secmon).elf
	$(OC) $^ -O binary $(secmon).bin

kernel: $(kernel).elf
	$(OC) $^ -O binary $(kernel).img

$(secmon).elf: $(OBJS_sec)
	$(LD) -m aarch64elf -nostdlib -T secmon/linker.ld -o $@ $^

$(kernel).elf: $(OBJS_kern)
	$(LD) -m aarch64elf -nostdlib -T kernel/linker.ld -o $@ $^

packKips: $(CURDIR)/KIPs/kip_blob
	$(OC) --update-section .kips=$^ $(kernel).elf
	$(OC) $(kernel).elf -O binary $(kernel).img

$(CURDIR)/KIPs/kip_blob:
	$(MAKE) -C KIPs

$(BUILD)/%.o: %.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

setup:
	mkdir -p $(BUILD)/kernel/memory
	mkdir -p $(BUILD)/secmon
	mkdir -p $(BUILD)/libraries
