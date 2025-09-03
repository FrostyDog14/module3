CC = gcc
CFLAGS = -Wall -Wextra -O2

all: server client1 client2

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client1: client1.c
	$(CC) $(CFLAGS) -o client1 client1.c

client2: client2.c
	$(CC) $(CFLAGS) -o client2 client2.c

clean:
	rm -f server client1 client2