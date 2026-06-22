CC     = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2

SDL_CFLAGS = -IC:/msys64/ucrt64/include
SDL_LIBS   = -LC:/msys64/ucrt64/lib -lmingw32 -mwindows -lSDL2main -lSDL2 -lSDL2_ttf

SRC = main.c \
      client/view/src/ui_welcome.c \
      client/view/src/ui_login.c \
      client/view/src/ui_chat.c

TARGET = myDiscord

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(SRC) -o $(TARGET) $(SDL_LIBS)

clean:
	del /f $(TARGET)

.PHONY: all clean