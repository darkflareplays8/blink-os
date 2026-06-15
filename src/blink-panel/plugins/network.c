#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib.h>
#include "../include/plugin.h"

static gboolean is_connected(void) {
    GNetworkMonitor *mon = g_network_monitor_get_default();
    return g_network_monitor_get_connectivity(mon) == G_NETWORK_CONNECTIVITY_FULL;
}

static gboolean update_network(gpointer data) {
    GtkImage *img = GTK_IMAGE(data);
    const gchar *icon = is_connected()
        ? "network-wireless-signal-good-symbolic"
        : "network-offline-symbolic";
    gtk_image_set_from_icon_name(img, icon);
    return G_SOURCE_CONTINUE;
}

static GtkWidget *create_widget(GKeyFile *config) {
    GtkWidget *btn = gtk_button_new();
    gtk_widget_add_css_class(btn, "blink-panel-btn");

    GtkWidget *img = gtk_image_new_from_icon_name("network-offline-symbolic");
    gtk_button_set_child(GTK_BUTTON(btn), img);

    update_network(img);
    g_timeout_add_seconds(5, update_network, img);

    g_signal_connect_swapped(btn, "clicked",
        G_CALLBACK(g_spawn_command_line_async),
        "nm-connection-editor");

    return btn;
}

static BlinkPluginInfo info = {
    .name           = "network",
    .description    = "Network status",
    .version        = "1.0",
    .create_widget  = create_widget,
    .destroy_widget = NULL,
};

BlinkPluginInfo *blink_plugin_get_info(void) { return &info; }
