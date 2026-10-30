Project := kern

boot   	:= boot
kernel 	:= kernel8
secmon	:= secmon

BUILD 	  := build
RESOURCES := resources
IMAGE_SIZE_MB := 64
PROC_DIR      := bin

#Pi4 firmware; start4.elf must come from the same release as fixup4.dat
FW_COMMIT := effea745e592ec8a97fc54093a5673a7e6e515c9
FW_URL    := https://raw.githubusercontent.com/raspberrypi/firmware/$(FW_COMMIT)/boot

export PROJECT_ROOT := $(CURDIR)

CC=clang
OC=llvm-objcopy
LD=ld.lld
INCLUDE := include
LIBS    := $(BUILD)/libraries/libraries.a
COMMON := -Wall -g -O0 -ffreestanding -nostdinc -nostdlib -march=armv8-a -mcpu=cortex-a72+nosimd

#qemu's bcm2838 GIC lacks the security extensions the real GIC-400 has; without
#them every access is treated as secure and NS IAR reads of grp1 irqs return 1022
QEMU_MACHINE := raspi4b -global arm_gic.has-security-extensions=on
ifeq ($(PLAT),PI3)
	COMMON += -DPI3
	QEMU_MACHINE := raspi3b
endif
QEMU_SECMON := $(BUILD)/$(secmon)_qemu.bin
QEMU_ARGS := -M $(QEMU_MACHINE) \
			-serial stdio \
			-device loader,file=$(QEMU_SECMON),addr=0x0,force-raw=on \
			-device loader,file=$(boot).bin,addr=0x80000,force-raw=on \
			-drive file=$(Project).img,if=sd,format=raw

CFLAGS := -I$(INCLUDE) $(COMMON) -mgeneral-regs-only
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

.PHONY: all clean library KIPs processes secmon kernel boot image qemu qemu-gdb

all: setup library KIPs processes secmon kernel boot image

clean:
	@rm -fr $(BUILD) $(kernel).elf $(secmon).elf $(boot).elf $(kernel).img $(secmon).bin $(boot).bin $(Project).img
	$(MAKE) -C KIPs clean
	$(MAKE) -C processes clean
	$(MAKE) -C secmon clean
	$(MAKE) -C libraries clean

qemu: $(QEMU_SECMON)
	qemu-system-aarch64 $(QEMU_ARGS)

qemu-gdb: $(QEMU_SECMON)
	qemu-system-aarch64 $(QEMU_ARGS) -S -s

#FW clears stub magic/version at 0xf0 (doubles as spin_cpu3) before releasing cores
$(QEMU_SECMON): $(secmon).bin
	cp $< $@
	dd if=/dev/zero of=$@ bs=1 seek=240 count=8 conv=notrunc status=none

#MBR w/ single FAT32 boot partition at 1MiB
image: $(RESOURCES)/bootfs/start4.elf
	dd if=/dev/zero of=$(Project).img bs=1M count=$(IMAGE_SIZE_MB)
	echo 'start=2048, type=c, bootable' | sfdisk -q $(Project).img
	mkfs.fat -F 32 -n "BOOT" --offset 2048 $(Project).img
	for file in $(RESOURCES)/bootfs/*; do \
		mcopy -s -i $(Project).img@@1M "$$file" "::/"; \
	done
	mcopy -i $(Project).img@@1M "boot.bin" "::/"
	mcopy -i $(Project).img@@1M "secmon.bin" "::/"
#Userland process binaries; init loads everything in here (see PROC_DIR in KIPs/init)
	mmd -i $(Project).img@@1M "::/$(PROC_DIR)"
	for d in processes/*/src; do \
		p=$$(basename $$(dirname $$d)); \
		mcopy -i $(Project).img@@1M "processes/$$p/$$p.bin" "::/$(PROC_DIR)/"; \
	done

$(RESOURCES)/bootfs/start4.elf:
	curl -fL -o $@.tmp $(FW_URL)/start4.elf
	mv $@.tmp $@

library:
	$(MAKE) -C libraries PLAT=$(PLAT)
	cp libraries/libraries.a $(BUILD)/libraries

KIPs:
	$(MAKE) -C KIPs PLAT=$(PLAT)

processes:
	$(MAKE) -C processes PLAT=$(PLAT)

secmon: 
	$(MAKE) -C secmon PLAT=$(PLAT)
	cp secmon/secmon.bin .

boot: $(boot).elf
	$(OC) $^ -O binary $(boot).bin

kernel: $(kernel).elf
	$(OC) $^ -O binary $(kernel).img

$(boot).elf: $(OBJS_boot) $(LIBS)
	$(LD) -m aarch64elf -nostdlib -T boot/linker.ld -o $@ $^

$(kernel).elf: $(OBJS_kern) $(LIBS)
	$(LD) -m aarch64elf -nostdlib -T kernel/linker.ld -o $@ $^

#boot.S .incbin's the kernel image, so rebuild it when the kernel changes
$(BUILD)/boot/boot.o: $(kernel).img

#kernel/boot.S .incbin's the KIP blob
$(BUILD)/kernel/boot.o: KIPs/kip_blob

$(BUILD)/%.o: %.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $<

setup:
	mkdir -p $(BUILD)/kernel/memory
	mkdir -p $(BUILD)/boot
	mkdir -p $(BUILD)/libraries
