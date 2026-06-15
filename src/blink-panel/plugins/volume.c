#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include "../include/plugin.h"

static gint get_volume(void) {
    FILE *f = popen("wpctl get-volume @DEFAULT_AUDIO_SINK@ 2>/dev/null | awk '{print int($2*100)}'", "r");
    if (!f) return -1;
    gint vol = -1;
    fscanf(f, "%d", &vol);
    pclose(f);
    return vol;
}

static void update_icon(GtkButton *btn) {
    gint vol = get_volume();
    const gchar *icon = "audio-volume-muted-symbolic";
    if (vol > 66)       icon = "audio-volume-high-symbolic";
    else if (vol > 33)  icon = "audio-volume-medium-symbolic";
    else if (vol > 0)   icon = "audio-volume-low-symbolic";

    GtkWidget *img = gtk_image_new_from_icon_name(icon);
    gtk_button_set_child(btn, img);
}

static void on_scroll(GtkEventControllerScroll *ctl, gdouble dx, gdouble dy, gpointer data) {
    gchar *cmd;
    if (dy < 0) cmd = g_strdup("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+");
    else         cmd = g_strdup("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-");
    system(cmd);
    g_free(cmd);
    update_icon(GTK_BUTTON(data));
}

static gboolean poll_volume(gpointer data) {
    update_icon(GTK_BUTTON(data));
    return G_SOURCE_CONTINUE;
}

static GtkWidget *create_widget(GKeyFile *config) {
    GtkWidget *btn = gtk_button_new();
    gtk_widget_add_css_class(btn, "blink-panel-btn");
    update_icon(GTK_BUTTON(btn));

    GtkEventController *scroll = gtk_event_controller_scroll_new(
        GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
    g_signal_connect(scroll, "scroll", G_CALLBACK(on_scroll), btn);
    gtk_widget_add_controller(btn, scroll);

    g_timeout_add_seconds(2, poll_volume, btn);
    return btn;
}

static BlinkPluginInfo info = {
    .name           = "volume",
    .description    = "Volume control",
    .version        = "1.0",
    .create_widget  = create_widget,
    .destroy_widget = NULL,
};

BlinkPluginInfo *blink_plugin_get_info(void) { return &info; }
