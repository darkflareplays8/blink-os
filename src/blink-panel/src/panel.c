#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <dlfcn.h>
#include <string.h>
#include "../include/panel.h"
#include "../include/plugin.h"

#define PANEL_HEIGHT 38
#define PLUGIN_DIR   "/usr/lib/blink/panel-plugins"

struct _BlinkPanel {
    GtkWidget      *window;
    GtkWidget      *box;
    GtkWidget      *left_box;
    GtkWidget      *center_box;
    GtkWidget      *right_box;
    GKeyFile       *config;
    GList          *plugins;
    gchar          *position;
};

static void apply_panel_css(BlinkPanel *p) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css,
        "window.blink-panel {"
        "  background-color: #0d0d14;"
        "  border-top: 1px solid #1e1e2e;"
        "}"
        ".blink-panel-btn {"
        "  background: none;"
        "  border: none;"
        "  border-radius: 6px;"
        "  color: #cdd6f4;"
        "  padding: 4px 10px;"
        "  font-size: 13px;"
        "}"
        ".blink-panel-btn:hover {"
        "  background-color: #1e1e2e;"
        "}"
        ".blink-separator {"
        "  background-color: #1e1e2e;"
        "  min-width: 1px;"
        "  margin: 6px 4px;"
        "}"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(css);
}

BlinkPanel *blink_panel_new(void) {
    BlinkPanel *p = g_new0(BlinkPanel, 1);
    p->config = g_key_file_new();
    p->position = g_strdup("bottom");
    return p;
}

void blink_panel_load_config(BlinkPanel *p) {
    gchar *path = g_build_filename(g_get_home_dir(), ".config", "blink", "panel.conf", NULL);
    GError *err = NULL;

    if (!g_key_file_load_from_file(p->config, path, G_KEY_FILE_KEEP_COMMENTS, &err)) {
        g_clear_error(&err);
        g_key_file_set_string(p->config, "panel", "position", "bottom");
        g_key_file_set_string(p->config, "panel", "plugins_left", "launcher,tasklist");
        g_key_file_set_string(p->config, "panel", "plugins_center", "");
        g_key_file_set_string(p->config, "panel", "plugins_right", "systray,volume,network,clock");
        g_key_file_set_integer(p->config, "panel", "height", PANEL_HEIGHT);

        gchar *dir = g_path_get_dirname(path);
        g_mkdir_with_parents(dir, 0755);
        g_free(dir);
        g_key_file_save_to_file(p->config, path, NULL);
    }

    g_free(p->position);
    p->position = g_key_file_get_string(p->config, "panel", "position", NULL);
    if (!p->position) p->position = g_strdup("bottom");
    g_free(path);
}

static void setup_window(BlinkPanel *p) {
    p->window = gtk_window_new();
    gtk_widget_add_css_class(p->window, "blink-panel");
    gtk_window_set_decorated(GTK_WINDOW(p->window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(p->window), FALSE);
    gtk_layer_shell_init_for_window(GTK_WINDOW(p->window));
    gtk_layer_shell_set_layer(GTK_WINDOW(p->window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_shell_auto_exclusive_zone_enable(GTK_WINDOW(p->window));

    gint height = g_key_file_get_integer(p->config, "panel", "height", NULL);
    if (height <= 0) height = PANEL_HEIGHT;

    if (g_strcmp0(p->position, "top") == 0) {
        gtk_layer_shell_set_anchor(GTK_WINDOW(p->window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    } else {
        gtk_layer_shell_set_anchor(GTK_WINDOW(p->window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    }
    gtk_layer_shell_set_anchor(GTK_WINDOW(p->window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_shell_set_anchor(GTK_WINDOW(p->window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_widget_set_size_request(p->window, -1, height);
}

static void load_plugin_group(BlinkPanel *p, const gchar *key, GtkWidget *box) {
    GError *err = NULL;
    gchar *val = g_key_file_get_string(p->config, "panel", key, &err);
    if (err || !val || strlen(val) == 0) { g_clear_error(&err); g_free(val); return; }

    gchar **names = g_strsplit(val, ",", -1);
    g_free(val);

    for (int i = 0; names[i]; i++) {
        g_strstrip(names[i]);
        if (strlen(names[i]) == 0) continue;

        gchar *sopath = g_strdup_printf("%s/blink-plugin-%s.so", PLUGIN_DIR, names[i]);
        void *handle = dlopen(sopath, RTLD_LAZY);
        g_free(sopath);

        if (!handle) {
            g_warning("Plugin not found: %s", names[i]);
            continue;
        }

        BlinkPluginInfo *(*get_info)(void) = dlsym(handle, "blink_plugin_get_info");
        if (!get_info) { dlclose(handle); continue; }

        BlinkPluginInfo *info = get_info();
        if (!info || !info->create_widget) { dlclose(handle); continue; }

        GtkWidget *widget = info->create_widget(p->config);
        if (widget) gtk_box_append(GTK_BOX(box), widget);

        BlinkLoadedPlugin *lp = g_new0(BlinkLoadedPlugin, 1);
        lp->handle = handle;
        lp->info = info;
        p->plugins = g_list_append(p->plugins, lp);
    }

    g_strfreev(names);
}

void blink_panel_load_plugins(BlinkPanel *p) {
    setup_window(p);
    apply_panel_css(p);

    p->box = gtk_center_box_new();
    gtk_widget_set_hexpand(p->box, TRUE);

    p->left_box   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    p->center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    p->right_box  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    gtk_widget_set_margin_start(p->left_box, 4);
    gtk_widget_set_margin_end(p->right_box, 4);

    gtk_center_box_set_start_widget(GTK_CENTER_BOX(p->box), p->left_box);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(p->box), p->center_box);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(p->box), p->right_box);

    load_plugin_group(p, "plugins_left",   p->left_box);
    load_plugin_group(p, "plugins_center", p->center_box);
    load_plugin_group(p, "plugins_right",  p->right_box);

    gtk_window_set_child(GTK_WINDOW(p->window), p->box);
}

void blink_panel_show(BlinkPanel *p) {
    gtk_widget_set_visible(p->window, TRUE);
}

void blink_panel_free(BlinkPanel *p) {
    for (GList *l = p->plugins; l; l = l->next) {
        BlinkLoadedPlugin *lp = l->data;
        dlclose(lp->handle);
        g_free(lp);
    }
    g_list_free(p->plugins);
    g_key_file_free(p->config);
    g_free(p->position);
    g_free(p);
}
