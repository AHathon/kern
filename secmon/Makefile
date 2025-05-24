target := secmon

BUILD  := build
SOURCE := src

CC=clang
OC=llvm-objcopy
LD=ld.lld
INCLUDE := include

COMMON := -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
CFLAGS := -I$(INCLUDE) $(COMMON)
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__ $(COMMON)

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S := $(call rwildcard, $(SOURCE), *.S) 
SRCS_C := $(call rwildcard, $(SOURCE), *.c) 
	
OBJS := $(patsubst $(SOURCE)/%.c, $(BUILD)/%.o, $(SRCS_C)) \
		$(patsubst $(SOURCE)/%.S, $(BUILD)/%.o, $(SRCS_S))

.PHONY: all clean secmon

all: setup secmon

clean:
	@rm -fr $(BUILD) $(target).elf $(target).bin

setup:
	mkdir -p $(BUILD)

secmon: $(target).elf
	$(OC) $^ -O binary $(target).bin

$(target).elf: $(OBJS)
	$(LD) -m aarch64elf -nostdlib -T linker.ld -o $@ $^

$(BUILD)/%.o: $(SOURCE)/%.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(SOURCE)/%.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^
