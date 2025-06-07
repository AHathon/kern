TARGET := libraries

BUILD := build

CC=clang
OC=llvm-objcopy
LD=ld.lld
AR=llvm-ar
INCLUDE := ../include

COMMON := -Wall -g -O0 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
CFLAGS := -I$(INCLUDE) $(COMMON)
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__ $(COMMON)

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S := $(call rwildcard, hardware, *.S) \
		  $(call rwildcard, services, *.S)

SRCS_C := $(call rwildcard, hardware, *.c) \
		  $(call rwildcard, services, *.c)

OBJS := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S))

.PHONY: all clean $(TARGET).a

all: setup $(TARGET).a

clean:
	@rm -fr $(BUILD) $(TARGET).a

$(TARGET).a: $(OBJS)
	$(AR) rc $@ $^

$(BUILD)/%.o: %.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

setup:
	mkdir -p $(BUILD)/hardware
	mkdir -p $(BUILD)/services
