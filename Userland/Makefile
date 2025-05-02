include Makefile.inc

SAMPLE_DATA=0001-sampleDataModule.bin
MODULES := $(wildcard *Module)

all: $(MODULES) sampleDataModule

$(MODULES): FORCE
	cd $@; make

sampleDataModule:
	printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

clean:
	@for module in $(MODULES); do \
		cd $$module; make clean; \
	done
	rm -rf *.bin *.elf

FORCE:

.PHONY: all clean
