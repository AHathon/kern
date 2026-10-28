#Userland (EL0) processes; not embedded in the kernel
DIRS := gfx

PROC_FILES := $(foreach d,$(DIRS),$(d)/$(d).kip)

.PHONY: all clean

all: $(PROC_FILES)

$(PROC_FILES): FORCE
	$(MAKE) -C $(@D)

FORCE:

clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
