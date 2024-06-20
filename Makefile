kernel := kernel.bin

INCLUDES = include/
linker_script := linker.ld
OBJS = $(addsuffix .o,$(patsubst %,bin/%,$(wildcard kernel/*.c libraries/*.c kernel/*.S libraries/*.S)))

.PHONY: all clean kernel qemu qemu-gdb

all: setup $(kernel)

clean:
	- @rm -fr bin $(kernel)

qemu: $(kernel)
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -kernel bin/$(kernel).elf

$(kernel): $(OBJS)
	aarch64-none-elf-g++ -T linker.ld -ffreestanding -O2 -nostdlib -lgcc -o bin/$(kernel).elf $^
	aarch64-none-elf-objcopy bin/$(kernel).elf -O binary kernel8.bin

# compile assembly files
bin/%.S.o: %.S
	aarch64-none-elf-gcc $(addprefix -I,$(INCLUDES)) -D__ASSEMBLY__ -c -o $@ $^

bin/%.c.o: %.c
	aarch64-none-elf-gcc $(addprefix -I,$(INCLUDES)) -ffreestanding -O2 -Wall -Wextra -c -o $@ $^

bin/%.cpp.o: %.cpp
	aarch64-none-elf-g++ $(addprefix -I,$(INCLUDES)) -ffreestanding -fno-exceptions -fno-rtti -O2 -Wall -Wextra -c -o $@ $^

setup:
	mkdir -p ./bin/kernel
