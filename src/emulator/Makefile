.PHONY: clean exec-build build libexecute libutils

CC       = gcc
CFLAGS   = -Wall -Wextra -g
CPPFLAGS = -I.. -I. -Iexecute -I../utils -MMD -MP

LDFLAGS = -Lexecute	-L../utils
LDLIBS  = -lexecute -lutils

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)

BUILD = emulate

build: libutils libexecute $(BUILD)

$(BUILD): $(OBJ) | libexecute libutils
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@
	cp $@ ../../../armv8_testsuite/solution

libexecute:
	$(MAKE) -C execute lib

libutils:
	$(MAKE) -C ../utils lib

-include $(DEP)

#%.o: %.c
#  $(CC) $(CFLAGS) $(CPPFLAGS) -C %< -o %@

clean:
	rm -f $(OBJ) $(BUILD) $(DEP)
	make -C execute clean
