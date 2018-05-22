CC=gcc
CFLAGS=-L
DEPS= big_mngr.h runs.h

BDIR = bin
SDIR = src

a.out: build

build: $(SDIR)/shell001.c clean
	mkdir $(BDIR)
	$(CC) $(SDIR)/shell001.c -o $(BDIR)/a.out $(CFLAGS) $(SDIR) -lreadline

clean:
	rm -rf bin
