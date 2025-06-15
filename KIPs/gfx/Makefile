TARGET := gfx
INCLUDE := -I$(PROJECT_ROOT)/include
LIBS := $(PROJECT_ROOT)/build/libraries/libraries.a

include $(PROJECT_ROOT)/misc/base_service_rules

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).bin $(TARGET)
