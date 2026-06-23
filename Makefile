CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -Iclient/view/include \
         `pkg-config --cflags sdl2 SDL2_ttf SDL2_image`
LIBS = `pkg-config --libs sdl2 SDL2_ttf SDL2_image`

SRC = main.c \
      client/view/src/ui_welcome.c \
      client/view/src/ui_login.c \
      client/view/src/ui_chat_store.c \
      client/view/src/ui_chat_components.c \
      client/view/src/ui_channels.c \
      client/view/src/ui_users.c \
      client/view/src/ui_chat.c

TARGET = myDiscord

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean