#Userland (EL0) processes; not embedded in the kernel
#Every <name>/src folder is built to <name>/<name>.bin and packed into /boot/bin for init to load
DIRS := $(patsubst %/src,%,$(wildcard */src))

PROC_FILES := $(foreach d,$(DIRS),$(d)/$(d).bin)

.PHONY: all clean

all: $(PROC_FILES)

$(PROC_FILES): FORCE
	$(MAKE) -C $(@D) -f $(PROJECT_ROOT)/misc/base_service_rules TARGET=$(@D) $(@F)

FORCE:

clean:
	for d in $(DIRS); do rm -rf $$d/build $$d/$$d.elf $$d/$$d.bin; done
