CC = gcc
OUTFILE = gomoku
CFLAGS = -g
LINKER_FLAGS =

.DEFAULT_GOAL := all

$(OUTFILE): main.o pai.o cli.o
	$(CC) $(LINKER_FLAGS) -o $@ $^

main.o: main.c gomoku.h
	$(CC) -c -o $@ $<

pai.o: pai.c pai.h gomoku.h
	$(CC) -c -o $@ $<

cli.o: cli.c cli.h gomoku.h
	$(CC) -c -o $@ $<

.PHONY: all
all: $(OUTFILE)

.PHONY: clean
clean:
	rm main.o pai.o cli.o $(OUTFILE)
