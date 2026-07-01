# MyDiscord

A homemade recreation of Discord, written in C with SDL2 for the interface and PostgreSQL for storage.
La Plateforme school project by Cecilia Perana, Daroueche Mari and Nelson Grac-Aubert.

## What you need installed

- MSYS2 (UCRT64 environment)
- PostgreSQL, running locally or on a machine you can reach

Everything else (SDL2, OpenSSL, libpq client headers) gets installed through MSYS2 below.

## Setting up MSYS2

Download and install MSYS2 from https://www.msys2.org if you don't have it yet.

Open the "MSYS2 UCRT64" terminal from your Start menu (not the plain MSYS2 one) and install the packages we need:

```
pacman -S mingw-w64-ucrt-x86_64-toolchain base-devel
pacman -S mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_image
pacman -S mingw-w64-ucrt-x86_64-postgresql mingw-w64-ucrt-x86_64-openssl
```

Then add MSYS2 to your Windows PATH so you can call `gcc` and `make` from anywhere: search "environment variables" in Windows, go to System variables, edit `Path`, and add:

```
C:\msys64\ucrt64\bin
```

Open a new terminal and check it worked:

```
gcc --version
make --version
```

If you're using VS Code and want a proper MSYS2 terminal inside it instead of switching windows all the time, add this to your user settings JSON (Ctrl+Shift+P, "Open User Settings JSON"):

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

Restart VS Code and you'll find "MSYS2 UCRT64" in the terminal dropdown.

## Setting up the database

You need a PostgreSQL server reachable from your machine, and an empty database called `mydiscord` (or whatever name you pick, just be consistent below).

Run the schema file against it:

```
psql -U postgres -d mydiscord -f data/myDiscord.sql
```

This creates all the tables and drops them first if they already exist, so don't run it against a database you care about without checking what's in `data/myDiscord.sql` first.

## Building

From the project root, build the client:

```
make
```

This produces `myDiscord.exe`.

Then build the server:

```
cd server
make
```

This produces `server/myDiscord_server.exe`.

## Running it

Start the server first, pointing it at your database. The connection string goes as the first argument:

```
cd server
./myDiscord_server.exe "host=localhost port=5432 dbname=mydiscord user=postgres password=yourpassword"
```

If you don't pass anything it falls back to `host=localhost port=5432 dbname=mydiscord user=postgres password=` with no password, which only works if your local Postgres is set up for that.

Once you see `listening on port 8080`, launch the client from the project root:

```
./myDiscord.exe
```

You can launch several clients on the same machine to test with multiple accounts, or have friends on the same wifi connect to you (see the LAN section below).

## Playing over LAN with friends

The server listens on all network interfaces, but the client is currently hardcoded to connect to `127.0.0.1`. To let someone else connect to your server:

1. Run `ipconfig` on your machine and note your IPv4 address on whatever network you're both on (a phone hotspot works well and avoids most school/office wifi restrictions).
2. Open `client/view/src/ui_welcome.c`, change the `SERVER_IP` constant near the top to your IP address, and rebuild (`make`).
3. Make sure your firewall lets through TCP port 8080, or just accept the Windows prompt the first time someone connects.

Your IP can change if your router hands out a new one, so it's worth checking again right before a demo rather than relying on what you noted down the day before.

## Using the app

### Creating an account and logging in

The first screen asks for an email, a username and a password to register, or just an email and password if you already have an account. Once you're in you land straight in the general channel.

### Channels

The left panel lists every channel you have access to. Public channels (the ones with a `#`) are visible to everyone. Private channels (marked with a lock) are only visible to whoever created them, at least for now.

Click the `+` at the top of the channel list to create a new one, give it a name and decide if it should be private.

To delete a channel, hover over it in the list and click the trash icon that appears. Only the person who created it (or an admin) can do that.

### Sending messages

Type in the box at the bottom and hit Enter, or click the send arrow. The box grows taller on its own if your message gets long. Messages show the sender's name and the time they were sent, oldest at the top like any normal chat.

If there are more messages than fit on screen, scroll with your mouse wheel. A scrollbar shows up on the right when there's something to scroll through.

To delete your own message, hover over it and click the trash icon.

### Reactions

Right click any message to react to it with a heart, a smiley or a sad face. If you already reacted, right clicking again lets you remove your reaction instead. When several people react the same way, you'll see a little counter next to the emoji instead of it repeating.

### Who's online

The right panel shows everyone online, and everyone else who has an account but isn't connected right now, further down. Moderators and admins show up with a small tag next to their name.

### Roles and moderation

There are three roles: user, moderator and admin. New accounts are always plain users, there's no way to promote yourself, someone has to change your role directly in the database.

If you're a moderator or an admin, right clicking someone in the online list gives you the option to ban them. Moderators can ban regular users, admins can also ban moderators, and nobody can ban an admin. Right clicking someone already banned gives you the option to unban them instead.

A ban stops someone from logging back in, it doesn't disconnect them if they're already connected.

### Calls and file transfer

The call button opens a basic call window. File transfer currently only works locally, meaning the file shows up in your own chat but doesn't actually get sent to anyone else yet.
