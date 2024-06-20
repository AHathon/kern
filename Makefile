kernel := kernel8

CC=aarch64-linux-gnu-gcc
OC=aarch64-linux-gnu-objcopy
INCLUDES = include/
linker_script := linker.ld
OBJS = $(addsuffix .o,$(patsubst %,bin/%,$(wildcard kernel/*.c libraries/*.c kernel/*.S libraries/*.S)))

.PHONY: all clean kernel qemu qemu-gdb

all: setup $(kernel).elf packKips

clean:
	@rm -fr bin $(kernel).elf
	$(MAKE) -C KIPs clean

qemu: $(kernel).elf
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel $(kernel).elf

$(kernel).elf: $(OBJS)
	$(CC) -T linker.ld -ffreestanding -O2 -nostdlib -lgcc -o $@ $^

packKips: $(CURDIR)/KIPs/kip_blob
	$(OC) --update-section .kips=$^ $(kernel).elf

$(CURDIR)/KIPs/kip_blob:
	$(MAKE) -C KIPs

# compile assembly files
bin/%.S.o: %.S
	$(CC) $(addprefix -I,$(INCLUDES)) -D__ASSEMBLY__ -c -o $@ $^

bin/%.c.o: %.c
	$(CC) $(addprefix -I,$(INCLUDES)) -ffreestanding -O2 -Wall -Wextra -c -o $@ $^

setup:
	mkdir -p ./bin/kernel
