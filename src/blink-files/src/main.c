#include <gtk/gtk.h>
#include <glib.h>
#include "../include/files.h"

static void on_activate(GApplication *app, gpointer data) {
    BlinkFiles *fm = blink_files_new(GTK_APPLICATION(app));
    blink_files_navigate(fm, g_get_home_dir());
    blink_files_show(fm);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("os.blink.files", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
