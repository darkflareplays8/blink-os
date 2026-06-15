#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#define BLINK_IPC_SOCKET "/tmp/blink-shell.sock"

static void send_ipc(const gchar *cmd) {
    GSocketClient *client = g_socket_client_new();
    GError *err = NULL;
    GSocketAddress *addr = g_unix_socket_address_new(BLINK_IPC_SOCKET);
    GSocketConnection *conn = g_socket_client_connect(client, G_SOCKET_CONNECTABLE(addr), NULL, &err);
    g_object_unref(addr);
    g_object_unref(client);
    if (err) { g_error_free(err); return; }
    GOutputStream *out = g_io_stream_get_output_stream(G_IO_STREAM(conn));
    g_output_stream_write(out, cmd, strlen(cmd), NULL, NULL);
    g_object_unref(conn);
}

static GKeyFile *load_config(void) {
    GKeyFile *kf = g_key_file_new();
    gchar *path = g_build_filename(g_get_home_dir(), ".config", "blink", "shell.conf", NULL);
    g_key_file_load_from_file(kf, path, G_KEY_FILE_KEEP_COMMENTS, NULL);
    g_free(path);
    return kf;
}

static void save_config(GKeyFile *kf) {
    gchar *path = g_build_filename(g_get_home_dir(), ".config", "blink", "shell.conf", NULL);
    g_key_file_save_to_file(kf, path, NULL);
    g_free(path);
}

static void apply_css(void) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css,
        "window { background-color: #0d0d14; color: #cdd6f4; }"
        "headerbar { background-color: #0a0a0f; border-bottom: 1px solid #1e1e2e; color: #cdd6f4; }"
        ".sidebar { background-color: #0a0a0f; border-right: 1px solid #1e1e2e; }"
        ".sidebar row { border-radius: 6px; margin: 2px 8px; padding: 8px; color: #cdd6f4; }"
        ".sidebar row:selected { background-color: #1e1e2e; }"
        ".settings-page { padding: 24px; }"
        ".settings-group { background-color: #1e1e2e; border-radius: 12px; padding: 16px; margin-bottom: 16px; }"
        ".settings-group label { color: #cdd6f4; }"
        ".settings-group-title { font-size: 11px; color: #6c7086; font-weight: bold; margin-bottom: 8px; }"
        "button.accent { background-color: #00d4ff; color: #0d0d14; border-radius: 8px; padding: 6px 16px; border: none; }"
        "button.accent:hover { background-color: #33deff; }"
        "entry { background-color: #313244; border: 1px solid #45475a; border-radius: 8px; color: #cdd6f4; padding: 6px 12px; }"
        "switch:checked { background-color: #00d4ff; }"
        "scale trough { background-color: #313244; }"
        "scale trough highlight { background-color: #00d4ff; }"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(css);
}

static GtkWidget *make_group(const gchar *title) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(box, "settings-group");
    if (title) {
        GtkWidget *lbl = gtk_label_new(title);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        gtk_widget_add_css_class(lbl, "settings-group-title");
        gtk_box_append(GTK_BOX(box), lbl);
    }
    return box;
}

static GtkWidget *make_row(const gchar *label, GtkWidget *control) {
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    GtkWidget *lbl = gtk_label_new(label);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0);
    gtk_widget_set_hexpand(lbl, TRUE);
    gtk_box_append(GTK_BOX(hbox), lbl);
    gtk_box_append(GTK_BOX(hbox), control);
    return hbox;
}

static GtkWidget *build_appearance_page(GKeyFile *kf) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(page, "settings-page");
    gtk_widget_set_vexpand(page, TRUE);

    GtkWidget *theme_group = make_group("THEME");

    const gchar *themes[] = { "blink-dark", "blink-light", "blink-midnight", "blink-ocean", NULL };
    GtkWidget *theme_combo = gtk_drop_down_new_from_strings(themes);
    gtk_box_append(GTK_BOX(theme_group), make_row("Theme", theme_combo));

    GtkWidget *accent_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    const gchar *accents[] = { "#00d4ff", "#cba6f7", "#a6e3a1", "#fab387", "#f38ba8", "#89b4fa" };
    for (int i = 0; accents[i]; i++) {
        GtkWidget *swatch = gtk_button_new();
        gtk_widget_set_size_request(swatch, 28, 28);
        GtkCssProvider *cp = gtk_css_provider_new();
        gchar *css = g_strdup_printf("button { background-color: %s; border-radius: 50%%; border: none; }", accents[i]);
        gtk_css_provider_load_from_string(cp, css);
        g_free(css);
        gtk_style_context_add_provider(gtk_widget_get_style_context(swatch),
            GTK_STYLE_PROVIDER(cp), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref(cp);
        g_object_set_data_full(G_OBJECT(swatch), "accent", g_strdup(accents[i]), g_free);
        g_signal_connect(swatch, "clicked", G_CALLBACK(lambda_set_accent), kf);
        gtk_box_append(GTK_BOX(accent_box), swatch);
    }
    gtk_box_append(GTK_BOX(theme_group), make_row("Accent colour", accent_box));

    GtkWidget *anim_sw = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(anim_sw),
        g_key_file_get_boolean(kf, "shell", "animations", NULL));
    gtk_box_append(GTK_BOX(theme_group), make_row("Animations", anim_sw));

    gtk_box_append(GTK_BOX(page), theme_group);

    GtkWidget *font_group = make_group("FONTS");
    GtkWidget *font_btn = gtk_font_dialog_button_new(gtk_font_dialog_new());
    gtk_box_append(GTK_BOX(font_group), make_row("Interface font", font_btn));
    gtk_box_append(GTK_BOX(page), font_group);

    GtkWidget *wp_group = make_group("WALLPAPER");
    GtkWidget *wp_btn = gtk_button_new_with_label("Choose wallpaper…");
    gtk_widget_add_css_class(wp_btn, "accent");
    gtk_box_append(GTK_BOX(wp_group), wp_btn);
    gtk_box_append(GTK_BOX(page), wp_group);

    return page;
}

static GtkWidget *build_display_page(void) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(page, "settings-page");

    GtkWidget *group = make_group("DISPLAY");

    GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 100, 200, 25);
    gtk_scale_add_mark(GTK_SCALE(scale), 100, GTK_POS_BOTTOM, "100%");
    gtk_scale_add_mark(GTK_SCALE(scale), 125, GTK_POS_BOTTOM, "125%");
    gtk_scale_add_mark(GTK_SCALE(scale), 150, GTK_POS_BOTTOM, "150%");
    gtk_scale_add_mark(GTK_SCALE(scale), 200, GTK_POS_BOTTOM, "200%");
    gtk_widget_set_hexpand(scale, TRUE);
    gtk_box_append(GTK_BOX(group), make_row("Scaling", scale));

    const gchar *refresh[] = { "60 Hz", "75 Hz", "120 Hz", "144 Hz", "165 Hz", "240 Hz", NULL };
    GtkWidget *refresh_combo = gtk_drop_down_new_from_strings(refresh);
    gtk_box_append(GTK_BOX(group), make_row("Refresh rate", refresh_combo));

    GtkWidget *night_sw = gtk_switch_new();
    gtk_box_append(GTK_BOX(group), make_row("Night light", night_sw));

    gtk_box_append(GTK_BOX(page), group);
    return page;
}

static GtkWidget *build_panel_page(void) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(page, "settings-page");

    GtkWidget *group = make_group("PANEL");

    const gchar *positions[] = { "Bottom", "Top", NULL };
    GtkWidget *pos_combo = gtk_drop_down_new_from_strings(positions);
    gtk_box_append(GTK_BOX(group), make_row("Position", pos_combo));

    GtkWidget *height_spin = gtk_spin_button_new_with_range(28, 64, 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(height_spin), 38);
    gtk_box_append(GTK_BOX(group), make_row("Height (px)", height_spin));

    GtkWidget *autohide_sw = gtk_switch_new();
    gtk_box_append(GTK_BOX(group), make_row("Auto-hide", autohide_sw));

    gtk_box_append(GTK_BOX(page), group);

    GtkWidget *plugins_group = make_group("PLUGINS");
    GtkWidget *plugins_lbl = gtk_label_new(
        "Edit ~/.config/blink/panel.conf to add or remove panel plugins.\n"
        "Available: clock, launcher, volume, network, tasklist, systray, battery, workspace");
    gtk_label_set_wrap(GTK_LABEL(plugins_lbl), TRUE);
    gtk_label_set_xalign(GTK_LABEL(plugins_lbl), 0);
    gtk_widget_add_css_class(plugins_lbl, "dim-label");
    gtk_box_append(GTK_BOX(plugins_group), plugins_lbl);
    gtk_box_append(GTK_BOX(page), plugins_group);

    return page;
}

static GtkWidget *build_about_page(void) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 24);
    gtk_widget_add_css_class(page, "settings-page");
    gtk_widget_set_valign(page, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(page, GTK_ALIGN_CENTER);

    GtkWidget *logo = gtk_image_new_from_icon_name("computer-symbolic");
    gtk_image_set_pixel_size(GTK_IMAGE(logo), 96);
    gtk_box_append(GTK_BOX(page), logo);

    GtkWidget *name = gtk_label_new("Blink OS");
    gtk_widget_add_css_class(name, "title-1");
    gtk_box_append(GTK_BOX(page), name);

    GtkWidget *ver = gtk_label_new("Version 1.0");
    gtk_widget_add_css_class(ver, "dim-label");
    gtk_box_append(GTK_BOX(page), ver);

    GtkWidget *desc = gtk_label_new("Fast. Clean. Yours.");
    gtk_box_append(GTK_BOX(page), desc);

    GtkWidget *link = gtk_link_button_new_with_label(
        "https://github.com/darkflareplays8/blink-os", "GitHub");
    gtk_box_append(GTK_BOX(page), link);

    return page;
}

typedef struct { GtkWidget *stack; GKeyFile *kf; } AppData;

static void on_sidebar_row(GtkListBox *lb, GtkListBoxRow *row, gpointer data) {
    AppData *d = data;
    const gchar *page = g_object_get_data(G_OBJECT(row), "page");
    if (page) gtk_stack_set_visible_child_name(GTK_STACK(d->stack), page);
}

static void on_activate(GApplication *app, gpointer data) {
    apply_css();

    GKeyFile *kf = load_config();
    AppData *d = g_new0(AppData, 1);
    d->kf = kf;

    GtkWidget *win = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(win), "Blink Settings");
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 540);

    GtkWidget *header = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(win), header);

    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(paned), 200);

    GtkWidget *sidebar = gtk_list_box_new();
    gtk_widget_add_css_class(sidebar, "sidebar");

    struct { const gchar *label; const gchar *icon; const gchar *page; } items[] = {
        { "Appearance", "preferences-desktop-theme-symbolic",    "appearance" },
        { "Display",    "video-display-symbolic",                "display"    },
        { "Panel",      "view-more-horizontal-symbolic",         "panel"      },
        { "About",      "help-about-symbolic",                   "about"      },
        { NULL }
    };

    for (int i = 0; items[i].label; i++) {
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_widget_set_margin_start(hbox, 4);
        GtkWidget *icon = gtk_image_new_from_icon_name(items[i].icon);
        GtkWidget *lbl  = gtk_label_new(items[i].label);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        gtk_widget_set_hexpand(lbl, TRUE);
        gtk_box_append(GTK_BOX(hbox), icon);
        gtk_box_append(GTK_BOX(hbox), lbl);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), hbox);
        g_object_set_data_full(G_OBJECT(row), "page", g_strdup(items[i].page), g_free);
        gtk_list_box_append(GTK_LIST_BOX(sidebar), row);
    }

    GtkWidget *sidebar_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sidebar_scroll), sidebar);
    gtk_widget_set_vexpand(sidebar_scroll, TRUE);
    gtk_paned_set_start_child(GTK_PANED(paned), sidebar_scroll);

    GtkWidget *stack = gtk_stack_new();
    d->stack = stack;
    gtk_stack_add_named(GTK_STACK(stack), build_appearance_page(kf), "appearance");
    gtk_stack_add_named(GTK_STACK(stack), build_display_page(),       "display");
    gtk_stack_add_named(GTK_STACK(stack), build_panel_page(),         "panel");
    gtk_stack_add_named(GTK_STACK(stack), build_about_page(),         "about");
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "appearance");
    gtk_paned_set_end_child(GTK_PANED(paned), stack);

    g_signal_connect(sidebar, "row-activated", G_CALLBACK(on_sidebar_row), d);

    gtk_window_set_child(GTK_WINDOW(win), paned);
    gtk_widget_set_visible(win, TRUE);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("os.blink.settings", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
