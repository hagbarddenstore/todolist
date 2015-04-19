TARGET=todo

CC=gcc

CFLAGS=-std=c99 -Wall -Isrc

LINKER=gcc -o

LFLAGS=-Wall -I. -lm -lsqlite3

SRCDIR=src
OBJDIR=obj
BINDIR=bin

SOURCES := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJECTS)
	rm -f $(BINDIR)/$(TARGET)
