#include <gtk/gtk.h>

static void on_activate(GtkApplication *app, gpointer data)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "MyDiscord");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("com.mydiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}