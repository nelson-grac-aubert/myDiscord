#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "db_init.h"

int main(int argc, char *argv[])
{
    const char *host   = argc > 1 ? argv[1] : "localhost";
    const char *port   = argc > 2 ? argv[2] : "5432";
    const char *dbname = argc > 3 ? argv[3] : "mydiscord";
    const char *user   = argc > 4 ? argv[4] : "postgres";
    const char *pass   = argc > 5 ? argv[5] : "";

    char conninfo[512];
    snprintf(conninfo, sizeof(conninfo),
             "host=%s port=%s dbname=%s user=%s password=%s",
             host, port, dbname, user, pass);

    printf("Connexion : %s\n", conninfo);

    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "[ERREUR] Connexion echouee : %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return EXIT_FAILURE;
    }
    printf("[OK] Connecte a %s\n\n", dbname);

    int ret = initializeData(conn);

    PQfinish(conn);
    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}