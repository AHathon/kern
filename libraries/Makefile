TARGET := libraries

BUILD := build

CC=clang
OC=llvm-objcopy
LD=ld.lld
AR=llvm-ar
INCLUDE := $(PROJECT_ROOT)/include

COMMON := -Wall -g -O0 -ffreestanding -nostdinc -nostdlib -march=armv8-a -mcpu=cortex-a72+nosimd

ifeq ($(PLAT),PI3)
	COMMON += -DPI3
endif

#Service clients pull protocol definitions from the services themselves
CFLAGS := -I$(INCLUDE) -I$(PROJECT_ROOT)/KIPs/fs/include -I$(PROJECT_ROOT)/processes/gfx/include -I$(PROJECT_ROOT)/processes/hid/include $(COMMON) -mgeneral-regs-only
ASFLAGS := -I$(INCLUDE) -D__ASSEMBLY__ $(COMMON)

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

SRCS_S := $(call rwildcard, hardware, *.S) 	\
		  $(call rwildcard, services, *.S) 	\
		  $(call rwildcard, ui, *.S)		\
		  $(call rwildcard, libc, *.S)		\
		  $(call rwildcard, ipc, *.S)

SRCS_C := $(call rwildcard, hardware, *.c) 	\
		  $(call rwildcard, services, *.c) 	\
		  $(call rwildcard, ui, *.c)		\
		  $(call rwildcard, libc, *.c)		\
		  $(call rwildcard, ipc, *.c)

OBJS := $(patsubst %.c, $(BUILD)/%.o, $(SRCS_C)) \
		$(patsubst %.S, $(BUILD)/%.o, $(SRCS_S))

.PHONY: all clean $(TARGET).a

all: setup $(TARGET).a

clean:
	@rm -fr $(BUILD) $(TARGET).a

$(TARGET).a: $(OBJS)
	$(AR) rc $@ $^

#math and stdio use FP registers, so they can't be general-regs-only; the kernel never links them
$(BUILD)/libc/%.o: CFLAGS += -O2
$(BUILD)/libc/math.o $(BUILD)/libc/stdio.o: CFLAGS := -I$(INCLUDE) $(COMMON) -O2

$(BUILD)/%.o: %.S
	$(CC) --target=aarch64-elf $(ASFLAGS) -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) --target=aarch64-elf $(CFLAGS) -c -o $@ $^

setup:
	mkdir -p $(BUILD)/hardware
	mkdir -p $(BUILD)/services
	mkdir -p $(BUILD)/ui
	mkdir -p $(BUILD)/libc
	mkdir -p $(BUILD)/ipc
