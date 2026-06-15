#pragma once
#include <gtk/gtk.h>
#include <glib.h>

typedef struct {
    const gchar  *name;
    const gchar  *description;
    const gchar  *version;
    GtkWidget   *(*create_widget)(GKeyFile *config);
    void         (*destroy_widget)(GtkWidget *w);
} BlinkPluginInfo;

typedef struct {
    void            *handle;
    BlinkPluginInfo *info;
} BlinkLoadedPlugin;

BlinkPluginInfo *blink_plugin_get_info(void);
