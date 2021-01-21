CFLAGS=-O2 -std=c99 -Wall
DESTDIR=/usr/local/bin
LDLIBS=-lSDL2

zorn: zorn.c
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

install:
	cp zorn $(DESTDIR)

clean:
	rm -rf zorn
