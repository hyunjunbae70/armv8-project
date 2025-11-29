.PHONY: clean all build libutils

CC       = gcc
CFLAGS   = -Wall -Wextra -g -pedantic
CPPFLAGS = -I.. -I. -I../utils -MMD -MP

LDFLAGS = -L../utils
LDLIBS  = -lutils

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)

BUILD = assemble

build: libutils $(BUILD)

$(BUILD): $(OBJ) | libutils
	$(CC) $(CFLAGS) $(CFLAGS) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@
	cp $@ ../../../armv8_testsuite/solution

# for subdirectories in the future
libutils:
	$(MAKE) -C../utils lib

-include $(DEP)

clean:
	rm -f $(OBJ) $(BUILD) $(DEP)
#	make -C execute clean
