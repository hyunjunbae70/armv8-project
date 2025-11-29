.PHONY: clean lib test 

CC       = gcc
CFLAGS   = -Wall -Wextra -g -pedantic    # warnings, debug, *optimisation*
CPPFLAGS = -I. -I.. -I../assembler -I../emulator -MMD -MP # search path + dependency flags 

SRC        := $(wildcard *.c)
ALL_OBJ    := $(SRC:.c=.o)
TEST_OBJ   := $(filter test%,$(ALL_OBJ))
BUILD_TEST := $(TEST_OBJ:.o=) 
OBJ        := $(filter-out test%,$(ALL_OBJ))
DEP        := $(ALL_OBJ:.o=.d)

AR      = ar
ARFLAGS = -rcs
LIB     = libutils.a

lib: $(LIB)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

-include $(DEP)

test: lib $(BUILD_TEST)

$(BUILD_TEST): %: %.o | lib
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -L. -lutils -o $@

clean:
	rm -f $(ALL_OBJ) $(DEP) $(BUILD_TEST) $(LIB)
