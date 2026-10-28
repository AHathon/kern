TARGET := kip_blob
DIRS := init fs pm

KIP_FILES := $(foreach d,$(DIRS),$(d)/$(d).kip)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(KIP_FILES)
	@cat $(KIP_FILES) > $@

$(KIP_FILES): FORCE
	$(MAKE) -C $(@D)

FORCE:

clean:
	@rm -f kip_blob
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
