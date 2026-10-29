TARGET := init
INCLUDE := -I$(PROJECT_ROOT)/KIPs/fs/include
LIBS :=

include $(PROJECT_ROOT)/misc/base_service_rules

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).kip $(TARGET)
