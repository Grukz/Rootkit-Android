# Compilation de fichiers lex autonomes (sans source C associÃ©).
LEX=flex
YACC=bison -d -v
CC=gcc
LDLIBS=-lfl
CFLAGS=-std=c99

LEX_FILES = $(wildcard *.l)
EXECUTABLES = $(patsubst %.l,%,$(LEX_FILES))

all: $(EXECUTABLES)

.PHONY: clean
clean:
	rm -f *.o core
	rm -f lex.yy.c
	rm $(EXECUTABLES)
