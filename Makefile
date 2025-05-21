boot   := boot
kernel := kernel8
secmon := secmon

BUILD := build

CC=clang
OC=llvm-objcopy
LD=ld.lld
INCLUDE := include

CFLAGS := -I$(INCLUDE) -Wall -g -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd -fPIE
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S_boot := $(call rwildcard, boot, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C_boot := $(call rwildcard, boot, *.c) \
		  $(call rwildcard, libraries, *.c)

SRCS_S_kern := $(call rwildcard, kernel, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C_kern := $(call rwildcard, kernel, *.c) \
		  $(call rwildcard, libraries, *.c)

SRCS_S_sec := $(call rwildcard, secmon, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C_sec := $(call rwildcard, secmon, *.c) \
		  $(call rwildcard, libraries, *.c)

OBJS_boot := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_boot)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_boot))

OBJS_kern := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_kern)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_kern))
	
OBJS_sec := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_sec)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_sec))

.PHONY: all clean kernel boot qemu qemu-gdb

all: setup secmon kernel boot

clean:
	@rm -fr $(BUILD) $(kernel).elf $(secmon).elf $(boot).elf $(kernel).img $(secmon).bin $(boot).bin
	$(MAKE) -C KIPs clean

qemu: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(boot).bin

qemu-gdb: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(boot).bin -s -S

boot: $(boot).elf
	$(OC) $^ -O binary $(boot).bin

secmon: $(secmon).elf
	$(OC) $^ -O binary $(secmon).bin

kernel: $(kernel).elf
	$(OC) $^ -O binary $(kernel).img

boot.elf: $(OBJS_boot)
	$(LD) -m aarch64elf -nostdlib -T boot/linker.ld -o $@ $^

$(secmon).elf: $(OBJS_sec)
	$(LD) -m aarch64elf -nostdlib -T secmon/linker.ld -o $@ $^

$(kernel).elf: $(OBJS_kern)
	$(LD) -m aarch64elf -nostdlib -T kernel/linker.ld -o $@ $^

$(BUILD)/%.o: %.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

setup:
	mkdir -p $(BUILD)/kernel/memory
	mkdir -p $(BUILD)/secmon
	mkdir -p $(BUILD)/boot
	mkdir -p $(BUILD)/libraries/hardware
	$(MAKE) -C KIPs
