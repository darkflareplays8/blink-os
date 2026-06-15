#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include "../include/panel.h"
#include "../include/plugin.h"

int main(int argc, char **argv) {
    gtk_init();

    BlinkPanel *panel = blink_panel_new();
    blink_panel_load_config(panel);
    blink_panel_load_plugins(panel);
    blink_panel_show(panel);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    blink_panel_free(panel);
    g_main_loop_unref(loop);
    return 0;
}
