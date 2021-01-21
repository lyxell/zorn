CFLAGS=-O2 -std=c99 -Wall
DESTDIR=/usr/local/bin

zorn: zorn.c
	$(CC) $(CFLAGS) $^ -lSDL2 -o $@

install:
	cp zorn $(DESTDIR)

clean:
	rm -rf zorn
