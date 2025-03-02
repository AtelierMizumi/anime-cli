CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Isrc
LIBS = -lcurl -ljson-c -lncurses
SRC = src/main.c src/ui.c src/api.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = anime-cli

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean