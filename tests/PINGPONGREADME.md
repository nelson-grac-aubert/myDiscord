# Test réseau PING/PONG

Vérifie que vos machines communiquent bien entre elles avant de commencer le vrai projet.

---

## Prérequis

### Installer gcc (MinGW)
Télécharge et installe [MinGW](https://sourceforge.net/projects/mingw/).  
Durant l'installation, coche `mingw32-gcc-g++`.

Ajoute au PATH : `C:\MinGW\bin`

### Installer make
```bash
winget install GnuWin32.Make
```

Ajoute au PATH : `C:\Program Files (x86)\GnuWin32\bin`

> **Modifier le PATH :**  
> Touche Windows → "variables d'environnement" → Variables d'environnement  
> → Variables système → Path → Modifier → Nouveau → coller le chemin → OK partout  
> → **Redémarrer le terminal**

Vérifie :
```bash
gcc --version
make --version
```

---

## Trouver son IP

```bash
ipconfig
```

Chercher la carte avec une passerelle (chez nous `10.10.0.1`). L'IP ressemble à `10.10.x.x`.

---

## Compiler

```bash
cd tests/pingpong
make
```

---

## Lancer le test

**Sur le PC qui fait serveur :**
```bash
server.exe
```

**Sur les autres PC :**
```bash
client.exe <IP_du_serveur>
# ex: client.exe 10.10.7.209
```

---

## Résultat attendu

**Serveur :**
```
Server listening on port 8080...
>> 10.10.5.33 connected
>> received: PING
>> sent: PONG
```

**Client :**
```
Connecting to 10.10.7.209:8080...
Connected!
Sent: PING
Received: PONG
```

---

## Ça ne marche pas ?

**`impossible de joindre l'hôte`** → ton IP n'est pas en `10.10.x.x`, tu n'es pas sur le bon réseau WiFi.

**`délai d'attente dépassé`** → le pare-feu bloque. Sur le PC serveur :

```
Touche Windows → "Pare-feu Windows Defender avec fonctions avancées de sécurité"
→ Règles de trafic entrant
→ "Partage de fichiers et d'imprimantes (Demande d'écho - Trafic entrant ICMPv4)"
→ Clic droit → Activer la règle
```

Puis autoriser le port 8080 :

```
Règles de trafic entrant → Nouvelle règle
→ Port → TCP → 8080
→ Autoriser la connexion → Cocher les 3 cases → Nom : "MyDiscord" → Terminer
```