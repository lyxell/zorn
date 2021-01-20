CFLAGS=-std=c99 -Wall

zorn: zorn.c
	$(CC) $(CFLAGS) $^ -lSDL2 -o $@

clean:
	rm -rf zorn
