CC = gcc

# Options de compilation et inclusion de SDL2 et SDL2_ttf via pkg-config
CFLAGS = -Wall -Wextra -std=c17 `pkg-config --cflags sdl2 SDL2_ttf`
LIBS = `pkg-config --libs sdl2 SDL2_ttf`

SRC = main.c client/view/src/welcome.c client/view/src/ui_login.c

TARGET = myDiscord

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)