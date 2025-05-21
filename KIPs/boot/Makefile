TARGET := boot

BUILD := build
SOURCES := src

CC=clang
OC=llvm-objcopy
CFLAGS := --target=aarch64-none-elf -mcpu=cortex-a53 -O2 -nostdlib -ffreestanding

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))
OBJECTS := $(patsubst $(SOURCES)/%.cpp, $(BUILD)/%.o, $(call rwildcard, $(SOURCES), *.cpp))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(TARGET).bin
	$(CC) -c -x assembler-with-cpp $(CFLAGS) -DSERVICE_MAGIC="\"$(TARGET)\"" -DSERVICE_BIN="\"$(TARGET).bin\"" ../../include/libraries/services/container.S -o $(BUILD)/container.o
	$(OC) -O binary $(BUILD)/container.o $(TARGET).kip

$(TARGET).bin: $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -T linker.ld $(CFLAGS) -o $(TARGET).elf $^
	$(OC) $(CURDIR)/$(TARGET).elf -O binary $@

$(BUILD)/%.o: $(SOURCES)/%.cpp
	@mkdir -p $(@D)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).bin $(TARGET)