#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /* Connection string passed as argument, or fallback to localhost for dev */
    const char *conninfo = argc > 1
        ? argv[1]
        : "host=localhost port=5432 dbname=mydiscord user=postgres password=";

    ServerState s;

    if (server_init(&s, SERVER_PORT, conninfo) != 0)
        return EXIT_FAILURE;

    server_run(&s); /* blocks until Ctrl+C */

    server_cleanup(&s);
    return EXIT_SUCCESS;
}