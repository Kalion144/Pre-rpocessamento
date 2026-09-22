CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

all: analisador

analisador: main.c preprocessador.c preprocessador.h lexico.c lexico.h
	$(CC) $(CFLAGS) main.c preprocessador.c lexico.c -o analisador

test: analisador
	sh testes/executar_testes.sh

clean:
	rm -f analisador

.PHONY: all test clean
