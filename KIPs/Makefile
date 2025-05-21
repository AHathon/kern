TARGET := kip_blob

.PHONY: all clean

all: $(TARGET)

$(TARGET):
	$(MAKE) -C pm
	$(MAKE) -C fs
	$(MAKE) -C boot
	@cat boot/boot.kip pm/pm.kip fs/fs.kip > kip_blob

clean:
	@rm -f kip_blob
	$(MAKE) -C pm clean
	$(MAKE) -C fs clean
	$(MAKE) -C boot clean