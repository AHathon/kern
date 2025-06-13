TARGET := kip_blob
DIRS := init fs pm gfx

KIP_FILES := $(foreach d,$(DIRS),$(d)/$(d).kip)

.PHONY: all clean

all: $(TARGET)

$(TARGET):
	for d in $(DIRS); do $(MAKE) -C $$d; done
	@cat $(KIP_FILES) > kip_blob

clean:
	@rm -f kip_blob
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
