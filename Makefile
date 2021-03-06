CC = clang
OUTFILE = gomoku
CFLAGS = -g -O3
LINKER_FLAGS = -lpthread

.DEFAULT_GOAL := all

$(OUTFILE): judge.o main.o pai.o cli.o hash.o ai.o
	$(CC) $(CFLAGS) $(LINKER_FLAGS) -o $@ $^

judge.o: judge.c judge.h gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

pai.o: pai.c pai.h gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

cli.o: cli.c cli.h gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

hash.o: hash.c hash.h gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

ai.o: ai.c ai.h gomoku.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all
all: $(OUTFILE)

.PHONY: clean
clean:
	rm main.o pai.o cli.o judge.o hash.o ai.o $(OUTFILE)
