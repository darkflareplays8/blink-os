#include <gtk/gtk.h>
#include <glib.h>
#include <time.h>
#include "../include/plugin.h"

static gboolean update_clock(gpointer data) {
    GtkLabel *label = GTK_LABEL(data);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    gchar buf[32];
    strftime(buf, sizeof(buf), "%H:%M  %a %d %b", t);
    gtk_label_set_text(label, buf);
    return G_SOURCE_CONTINUE;
}

static GtkWidget *create_widget(GKeyFile *config) {
    GtkWidget *label = gtk_label_new("");
    gtk_widget_add_css_class(label, "blink-panel-btn");
    update_clock(label);
    g_timeout_add_seconds(1, update_clock, label);
    return label;
}

static BlinkPluginInfo info = {
    .name           = "clock",
    .description    = "Date and time display",
    .version        = "1.0",
    .create_widget  = create_widget,
    .destroy_widget = NULL,
};

BlinkPluginInfo *blink_plugin_get_info(void) { return &info; }
