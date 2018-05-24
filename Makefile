CC = gcc
CFLAGS = -lgmp -lbsd

all: dirs execs

.PHONY: dirs
dirs:
	@mkdir -p bin

execs: bin/decrypt_key bin/encrypt_key bin/square_tester

bin/decrypt_key: src/decrypt_key.c
	$(CC) $< -o $@ $(CFLAGS)

bin/encrypt_key: src/encrypt_key.c
	$(CC) $< -o $@ $(CFLAGS)

bin/square_tester: src/square_tester.c
	$(CC) $< -o $@ $(CFLAGS) 

.PHONY: clean
clean:
	rm -rf bin