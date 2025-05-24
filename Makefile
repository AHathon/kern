boot   := boot
kernel := kernel8

BUILD := build

CC=clang
OC=llvm-objcopy
LD=ld.lld
INCLUDE := include

COMMON := -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
CFLAGS := -I$(INCLUDE) $(COMMON)
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__ $(COMMON)

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S_boot := $(call rwildcard, boot, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C_boot := $(call rwildcard, boot, *.c) \
		  $(call rwildcard, libraries, *.c)

SRCS_S_kern := $(call rwildcard, kernel, *.S) \
		  $(call rwildcard, libraries, *.S)

SRCS_C_kern := $(call rwildcard, kernel, *.c) \
		  $(call rwildcard, libraries, *.c)

OBJS_boot := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_boot)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_boot))

OBJS_kern := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C_kern)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S_kern))

.PHONY: all clean secmon kernel boot qemu qemu-gdb

all: setup secmon kernel boot

clean:
	@rm -fr $(BUILD) $(kernel).elf $(secmon).elf $(boot).elf $(kernel).img $(secmon).bin $(boot).bin
	$(MAKE) -C KIPs clean
	$(MAKE) -C secmon clean

qemu: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(boot).bin

qemu-gdb: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(boot).bin -s -S

secmon: 
	$(MAKE) -C secmon

boot: $(boot).elf
	$(OC) $^ -O binary $(boot).bin

kernel: $(kernel).elf
	$(OC) $^ -O binary $(kernel).img

$(boot).elf: $(OBJS_boot)
	$(LD) -m aarch64elf -nostdlib -T boot/linker.ld -o $@ $^

$(kernel).elf: $(OBJS_kern)
	$(LD) -m aarch64elf -nostdlib -T kernel/linker.ld -o $@ $^

$(BUILD)/kernel/%.o: kernel/%.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/kernel/%.o: kernel/%.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

$(BUILD)/boot/%.o: boot/%.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/boot/%.o: boot/%.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

$(BUILD)/libraries/%.o: libraries/%.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/libraries/%.o: libraries/%.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

setup:
	mkdir -p $(BUILD)/kernel/memory
	mkdir -p $(BUILD)/boot
	mkdir -p $(BUILD)/libraries/hardware
	$(MAKE) -C KIPs
