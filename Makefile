CC = gcc
CFLAGS = -Wall -Wextra -I.
LIBS = -lcurl -ljson-c -lncurses

SRC = src/main.c \
	src/config.c \
	src/api/api.c \
	src/api/anime.c \
	src/api/manga.c \
	src/api/providers/aniwatch.c \
	src/api/providers/zoro.c \
	src/api/providers/mangadex.c \
	src/ui/ui.c \
	src/ui/anime_ui.c \
	src/ui/manga_ui.c \
	src/ui/common/input.c \
	src/ui/common/display.c \
	src/utils/memory.c \
	src/utils/string.c

OBJ = $(SRC:.c=.o)
TARGET = anime-cli

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

# Add this target for CI
dist: $(TARGET)
	mkdir -p dist
	cp $(TARGET) README.md LICENSE dist/
	tar -czvf anime-cli.tar.gz -C dist .

.PHONY: all clean rebuild dist