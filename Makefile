CC = gcc
CFLAGS = -Wall -Wextra -std=c17 \
         -Iclient/view/include \
         -IC:/msys64/ucrt64/include

LIBS = -LC:/msys64/ucrt64/lib \
       -lmingw32 -mwindows \
       -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image

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
	del /f $(TARGET)

.PHONY: all clean