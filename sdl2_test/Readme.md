## Prérequis & Installation

### 1. Installation des outils via MSYS2

Ouvrez le terminal **MSYS2 UCRT64** et exécutez la commande suivante pour installer le compilateur, l'outil Make et les bibliothèques graphiques nécessaires :

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf

```

### 2. Configuration du dossier de projet (Sous PowerShell)

Ouvrez votre terminal **PowerShell** dans le dossier de votre projet.

Pour que l'exécutable puisse se lancer sans erreur "DLL manquante", copiez l'ensemble des bibliothèques dynamiques requises depuis le dossier d'installation de MSYS2 vers votre répertoire actuel avec cette commande :

```powershell
cp C:\\msys64\\ucrt64\\bin\\*.dll .

```

*Note : Assurez-vous également que votre fichier de police de caractères est présent dans ce dossier et nommé correctement (ex: `font.ttf`).*

## Compilation et Exécution (PowerShell)

### 🛠️ Compiler le projet

Pour générer l'exécutable `sdl_test.exe`, lancez simplement :

```powershell
make

```

### 🚀 Lancer l'application

```powershell
.\sdl_test.exe

```

### 🧹 Nettoyer le projet

Pour supprimer l'exécutable et recompiler à blanc :

```powershell
make clean

```

## Fonctionnement du calcul de centrage

Pour positionner le texte mathématiquement au centre exact de la fenêtre, le programme récupère les dimensions de la surface textuelle générée par `SDL_ttf` et applique les formules de décalage suivantes :

$$X_{texte} = \frac{\text{Largeur Fenêtre} - \text{Largeur Texte}}{2}$$

$$Y_{texte} = \frac{\text{Hauteur Fenêtre} - \text{Hauteur Texte}}{2}$$


"""

```

```
