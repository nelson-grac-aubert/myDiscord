CC      = gcc
CFLAGS  = -Wall -Wextra
LDFLAGS = -lws2_32

all: server.exe client.exe

server.exe: server.c
	$(CC) $(CFLAGS) -o server.exe server.c $(LDFLAGS)

client.exe: client.c
	$(CC) $(CFLAGS) -o client.exe client.c $(LDFLAGS)

clean:
	del /Q *.exe