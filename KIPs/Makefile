TARGET := kip_blob

.PHONY: all clean

all: $(TARGET)

$(TARGET):
	$(MAKE) -C pm
	$(MAKE) -C fs
	@cat pm/pm fs/fs > kip_blob

clean:
	@rm -f kip_blob
	$(MAKE) -C pm clean
	$(MAKE) -C fs clean