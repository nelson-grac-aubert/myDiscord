CC = gcc
CFLAGS = -Wall -Wextra -std=c17 \
         -Iclient/view/include \
         -Iclient/controller/include \
         -Iclient/model/include \
         -IC:/msys64/ucrt64/include

LIBS = -LC:/msys64/ucrt64/lib \
       -lmingw32 -mwindows \
       -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image

# 📂 Liste des fichiers sources avec le bon chemin vers controller/src/ 🌟
SRC = main.c \
      client/view/src/ui_welcome.c \
      client/view/src/ui_login.c \
      client/view/src/ui_channels.c \
      client/view/src/ui_users.c \
      client/view/src/ui_chat.c \
      client/view/src/variables.c \
      client/controller/src/chat_controller.c \
      client/model/src/channel.c \
      client/model/src/message.c \
      client/model/src/user.c \
      client/model/src/reaction.c

TARGET = myDiscord.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean