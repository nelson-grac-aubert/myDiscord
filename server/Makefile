CC     = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2 -I./include -IC:/msys64/ucrt64/include
LIBS   = -LC:/msys64/ucrt64/lib -lpq -lws2_32

SRC = src/server_main.c \
      src/server.c \
      src/packet.c

TARGET = myDiscord_server.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	del /f $(TARGET)

.PHONY: all clean