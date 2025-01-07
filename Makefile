EXEC=parser.out
BIN=bin/
CC=cc
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.o,$(BIN)%.o,$(SOURCES:.c=.o))
INCLUDES=includes/
CFLAGS=-Wall -Wextra -Wconversion -Wno-missing-braces -pedantic -fno-strict-aliasing  -std=c11 -I$(INCLUDES)

ifeq ($(DEBUG), 1)
CFLAGS += -ggdb
endif

ifeq ($(LOG),1)
CFLAGS += -DLOG
endif 

ifeq ($(W64),1)
CC = x86_64-w64-mingw32-gcc
EXEC = fu.exe
endif 

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(BIN)$(EXEC) 

$(BIN)%.o: %.c 
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -rf $(BIN)

.PHONY: all clean