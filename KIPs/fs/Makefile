TARGET := fs

BUILD := build
SOURCES := src

CC=aarch64-linux-gnu-g++
OC=aarch64-linux-gnu-objcopy
CFLAGS := -mcpu=cortex-a53 -O2 -nostdlib -no-pie

rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))
OBJECTS := $(patsubst $(SOURCES)/%.cpp, $(BUILD)/%.o, $(call rwildcard, $(SOURCES), *.cpp))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -T linker.ld $(CFLAGS) -o $(TARGET).elf $^
	$(OC) $(CURDIR)/$(TARGET).elf -O binary $@

$(BUILD)/%.o: $(SOURCES)/%.cpp
	@mkdir -p $(@D)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET)
