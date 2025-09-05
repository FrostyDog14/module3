CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lrt

TARGET = taxi
SRC = taxi.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
