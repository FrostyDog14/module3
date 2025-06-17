CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SRCS = main.c person.c list.c anthroponym.c work.c

OBJS = $(SRCS:.c=.o)

TARGET = phonebook

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean 