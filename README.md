# MyDiscord 💬

> A recreation of the famous multi-channel chat server & app — written in C.  
> La Plateforme School Project by **Cecilia Perana**, **Daroueche Mari** and **Nelson Grac-Aubert**

---

## Dependencies

- **GTK4** — C GUI library
- **GCC** via MSYS2 UCRT64 toolchain
- **pkg-config**

---

## Installation

### 1. Install MSYS2

Download and install MSYS2 from https://www.msys2.org

### 2. Install GTK4 and the C toolchain

Open the **MSYS2 UCRT64** shell from the Start menu and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gtk4
pacman -S mingw-w64-ucrt-x86_64-toolchain base-devel
```

### 3. Add MSYS2 to the Windows PATH

In Windows, search for **"environment variables"** → System variables → `Path` → Edit → Add:

```
C:\msys64\ucrt64\bin
```

### 4. Verify the installation

Open a new terminal and run:

```bash
gcc --version
pkg-config --modversion gtk4
```

Both commands should return a version number without errors.

---

## VS Code setup

To open a MSYS2 UCRT64 terminal directly inside VS Code:

Open your user settings JSON (`Ctrl+Shift+P` → `Open User Settings JSON`) and add:

```json
"terminal.integrated.profiles.windows": {
    "MSYS2 UCRT64": {
        "path": "C:\\msys64\\usr\\bin\\bash.exe",
        "args": ["--login", "-i"],
        "env": {
            "MSYSTEM": "UCRT64",
            "CHERE_INVOKING": "1"
        }
    }
}
```

Save and restart VS Code. You can now open a MSYS2 UCRT64 terminal via the `+` dropdown in the terminal panel.

---

## Build & run

> Make sure you are in a **MSYS2 UCRT64** terminal, not PowerShell.

Switch to the `hello_world_test` branch first:

```bash
git checkout hello_world_test
```

Compile:

```bash
gcc main.c $(pkg-config --cflags --libs gtk4) -o mydiscord.exe
```

Run:

```bash
./mydiscord.exe
```
