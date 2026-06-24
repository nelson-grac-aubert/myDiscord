CC = gcc
CFLAGS = -Wall -Wextra -std=c17 \
         -Iclient/view/include \
         -Iclient/model/include \
         -IC:/msys64/ucrt64/include

LIBS = -LC:/msys64/ucrt64/lib \
       -lmingw32 -mwindows \
       -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image

# 📂 Liste des fichiers sources (Modèles inclus avec les bons noms de fichiers)
SRC = main.c \
      client/view/src/ui_welcome.c \
      client/view/src/ui_login.c \
      client/view/src/ui_channels.c \
      client/view/src/ui_users.c \
      client/view/src/ui_chat.c \
      client/model/src/channel.c \
      client/model/src/message.c \
      client/model/src/user.c \
      client/model/src/reaction.c

# 💡 Extension .exe pour Windows
TARGET = myDiscord.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

clean:
# ➡️ Conforme pour le terminal UCRT64 (Bash)
	rm -f $(TARGET)

.PHONY: all clean