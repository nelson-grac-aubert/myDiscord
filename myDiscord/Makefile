CC = gcc

# Options de compilation et inclusion de SDL2, SDL2_ttf et SDL2_image via pkg-config
# -Iclient/view/include permet au compilateur de trouver tes fichiers .h
CFLAGS = -Wall -Wextra -std=c17 -Iclient/view/include `pkg-config --cflags sdl2 SDL2_ttf SDL2_image`
LIBS = `pkg-config --libs sdl2 SDL2_ttf SDL2_image`

# Liste complète de tous les fichiers sources du projet
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
	$(CC) $(SRC) $(CFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)