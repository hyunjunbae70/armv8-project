.PHONY: all clean rebuild emulator assembler

BUILD = emulator assembler

# just run `make` to run `make all`
all: $(BUILD)

rebuild: clean all

$(BUILD):
	$(MAKE) -C $@ build

clean:
	$(MAKE) -C emulator clean
	$(MAKE) -C assembler clean
	$(MAKE) -C utils clean
