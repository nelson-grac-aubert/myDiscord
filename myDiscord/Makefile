CC     = gcc
CFLAGS = -Wall -Wextra -std=c17 `pkg-config --cflags sdl2 SDL2_ttf`
LIBS   = `pkg-config --libs sdl2 SDL2_ttf` -lws2_32

SRC = main.c \
      client/view/src/ui_welcome.c \
      client/view/src/ui_login.c \
      client/view/src/ui_chat.c \
      client/network/src/packet.c \
      client/network/src/client_socket.c

TARGET = myDiscord

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)