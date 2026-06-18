# Nom de l'exécutable final
TARGET = sdl_test.exe

# Compilateur à utiliser
CC = gcc

# Options de compilation (gestion des erreurs et avertissements)
CFLAGS = -Wall -Wextra -O2

# Chemins écrits "en dur" pour Windows/PowerShell (UCRT64)
# -I indique où trouver les headers (.h) de SDL2 et SDL_ttf
SDL_CFLAGS = -IC:/msys64/ucrt64/include

# -L indique où trouver les bibliothèques (.a / .dll) 
# On ajoute bien -lSDL2_ttf à la fin
SDL_LIBS   = -LC:/msys64/ucrt64/lib -lmingw32 -mwindows -lSDL2main -lSDL2 -lSDL2_ttf

# Règle par défaut (appelée quand on tape juste 'make')
all: $(TARGET)

# Règle de compilation de l'exécutable
$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET) $(SDL_CFLAGS) $(SDL_LIBS)

# Règle pour nettoyer les fichiers générés (version Windows PowerShell/CMD)
clean:
	del /f $(TARGET)

# Évite les conflits si des dossiers nommés 'all' ou 'clean' existent
.PHONY: all clean