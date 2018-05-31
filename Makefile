CC = gcc
CFLAGS = -lgmp -lbsd

all: dirs execs

.PHONY: dirs
dirs:
	@mkdir -p bin build

execs: bin/decrypt_key bin/encrypt_key bin/square_tester

bin/decrypt_key: src/decrypt_key.c build/math.o
	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) $< -o $@ $(CFLAGS)

bin/encrypt_key: src/encrypt_key.c build/math.o
	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) $^ -o $@ $(CFLAGS)

bin/square_tester: src/square_tester.c build/math.o
	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) $^ -o $@ $(CFLAGS) 

build/math.o: src/math.c src/math.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf bin build