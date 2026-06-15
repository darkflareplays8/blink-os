#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib.h>
#include <string.h>
#include "../include/plugin.h"

typedef struct {
    GtkWidget   *popover;
    GtkWidget   *search;
    GtkWidget   *list;
    GList       *apps;
} LauncherData;

static void populate_apps(LauncherData *d, const gchar *filter) {
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(d->list)))
        gtk_list_box_remove(GTK_LIST_BOX(d->list), child);

    GList *app_list = g_app_info_get_all();
    for (GList *l = app_list; l; l = l->next) {
        GAppInfo *app = l->data;
        if (!g_app_info_should_show(app)) continue;

        const gchar *name = g_app_info_get_name(app);
        if (filter && strlen(filter) > 0) {
            gchar *lower_name = g_utf8_strdown(name, -1);
            gchar *lower_filter = g_utf8_strdown(filter, -1);
            gboolean match = g_str_has_prefix(lower_name, lower_filter) ||
                             strstr(lower_name, lower_filter);
            g_free(lower_name);
            g_free(lower_filter);
            if (!match) continue;
        }

        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_widget_set_margin_start(hbox, 8);
        gtk_widget_set_margin_end(hbox, 8);
        gtk_widget_set_margin_top(hbox, 6);
        gtk_widget_set_margin_bottom(hbox, 6);

        GIcon *icon = g_app_info_get_icon(app);
        if (icon) {
            GtkWidget *img = gtk_image_new_from_gicon(icon);
            gtk_image_set_pixel_size(GTK_IMAGE(img), 24);
            gtk_box_append(GTK_BOX(hbox), img);
        }

        GtkWidget *label = gtk_label_new(name);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_box_append(GTK_BOX(hbox), label);

        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), hbox);
        g_object_set_data_full(G_OBJECT(row), "app-info", g_object_ref(app), g_object_unref);
        gtk_list_box_append(GTK_LIST_BOX(d->list), row);
    }
    g_list_free_full(app_list, g_object_unref);
}

static void on_search_changed(GtkSearchEntry *entry, gpointer data) {
    LauncherData *d = data;
    populate_apps(d, gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static void on_row_activated(GtkListBox *lb, GtkListBoxRow *row, gpointer data) {
    LauncherData *d = data;
    GAppInfo *app = g_object_get_data(G_OBJECT(row), "app-info");
    if (app) {
        GError *err = NULL;
        g_app_info_launch(app, NULL, NULL, &err);
        if (err) { g_warning("Launch failed: %s", err->message); g_error_free(err); }
    }
    gtk_popover_popdown(GTK_POPOVER(d->popover));
}

static void on_btn_clicked(GtkButton *btn, gpointer data) {
    LauncherData *d = data;
    gtk_editable_set_text(GTK_EDITABLE(d->search), "");
    populate_apps(d, "");
    gtk_popover_popup(GTK_POPOVER(d->popover));
    gtk_widget_grab_focus(d->search);
}

static GtkWidget *create_widget(GKeyFile *config) {
    LauncherData *d = g_new0(LauncherData, 1);

    GtkWidget *btn = gtk_button_new_with_label("  Blink");
    gtk_widget_add_css_class(btn, "blink-panel-btn");

    d->popover = gtk_popover_new();
    gtk_widget_set_parent(d->popover, btn);
    gtk_popover_set_has_arrow(GTK_POPOVER(d->popover), FALSE);
    gtk_popover_set_position(GTK_POPOVER(d->popover), GTK_POS_TOP);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_start(vbox, 8);
    gtk_widget_set_margin_end(vbox, 8);
    gtk_widget_set_margin_top(vbox, 8);
    gtk_widget_set_margin_bottom(vbox, 8);
    gtk_widget_set_size_request(vbox, 280, 400);

    d->search = gtk_search_entry_new();
    gtk_box_append(GTK_BOX(vbox), d->search);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    d->list = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), d->list);
    gtk_box_append(GTK_BOX(vbox), scroll);

    gtk_popover_set_child(GTK_POPOVER(d->popover), vbox);

    g_signal_connect(d->search, "search-changed", G_CALLBACK(on_search_changed), d);
    g_signal_connect(d->list, "row-activated", G_CALLBACK(on_row_activated), d);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_btn_clicked), d);

    return btn;
}

static BlinkPluginInfo info = {
    .name           = "launcher",
    .description    = "App launcher",
    .version        = "1.0",
    .create_widget  = create_widget,
    .destroy_widget = NULL,
};

BlinkPluginInfo *blink_plugin_get_info(void) { return &info; }
