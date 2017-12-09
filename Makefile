CC = gcc
OUTFILE = gomoku
CFLAGS = -g
LINKER_FLAGS =

.DEFAULT_GOAL := all

$(OUTFILE): main.o pai.o cli.o
	$(CC) $(CFLAGS) $(LINKER_FLAGS) -o $@ $^

main.o: main.c gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

pai.o: pai.c pai.h gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

cli.o: cli.c cli.h gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all
all: $(OUTFILE)

.PHONY: clean
clean:
	rm main.o pai.o cli.o $(OUTFILE)