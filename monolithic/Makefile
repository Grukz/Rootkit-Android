EXEC:=$(basename $(wildcard *.c) $(wildcard *.s))
EXEC:=${EXEC:manipmem=}
CC=arm-linux-gnueabi-gcc 
CFLAGS=-march=armv7
LDFLAGS=-static 

all: $(EXEC)

rootkit: rootkit.o manipmem.o

%:%.c

%:%.s
	$(CC) $(CFLAGS) $(LDFLAGS) -nostdlib -o $@ $<

.PHONY: clean
clean:
	@rm -f $(EXEC) *.o
