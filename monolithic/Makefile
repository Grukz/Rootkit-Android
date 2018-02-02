EXEC:=$(basename $(wildcard *.c) $(wildcard *.s))
EXEC:=${EXEC:manipmem=}
CC=gcc
CFLAGS=
LDFLAGS=-static

all: $(EXEC)

rootkit: rootkit.o manipmem.o

%:%.c

%:%.s
	$(CC) $(CFLAGS) -nostdlib -o $@ $<

.PHONY: clean
clean:
	@rm -f $(EXEC) *.o
