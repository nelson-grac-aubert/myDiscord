#include "client/view/include/welcome.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // Lance l'interface en mode SDL2
    welcome_ui_init_and_run();

    return 0;
}