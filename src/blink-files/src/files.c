#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib.h>
#include <string.h>
#include "../include/files.h"

#define ICON_SIZE 48

struct _BlinkFiles {
    GtkApplication  *app;
    GtkWidget       *window;
    GtkWidget       *header;
    GtkWidget       *back_btn;
    GtkWidget       *forward_btn;
    GtkWidget       *up_btn;
    GtkWidget       *path_entry;
    GtkWidget       *search_btn;
    GtkWidget       *paned;
    GtkWidget       *sidebar;
    GtkWidget       *view_stack;
    GtkWidget       *grid_view;
    GtkWidget       *list_view;
    GtkWidget       *status_bar;
    GtkWidget       *view_toggle;
    GList           *history;
    gint             history_pos;
    gchar           *current_path;
    GListStore      *store;
    gboolean         show_hidden;
    gboolean         grid_mode;
};

typedef struct {
    GObject   parent;
    GFileInfo *info;
    GFile     *file;
    gchar     *path;
} BlinkFileItem;

#define BLINK_TYPE_FILE_ITEM (blink_file_item_get_type())
G_DECLARE_FINAL_TYPE(BlinkFileItem, blink_file_item, BLINK, FILE_ITEM, GObject)
G_DEFINE_TYPE(BlinkFileItem, blink_file_item, G_TYPE_OBJECT)

static void blink_file_item_finalize(GObject *obj) {
    BlinkFileItem *item = BLINK_FILE_ITEM(obj);
    g_clear_object(&item->info);
    g_clear_object(&item->file);
    g_free(item->path);
    G_OBJECT_CLASS(blink_file_item_parent_class)->finalize(obj);
}

static void blink_file_item_class_init(BlinkFileItemClass *klass) {
    G_OBJECT_CLASS(klass)->finalize = blink_file_item_finalize;
}

static void blink_file_item_init(BlinkFileItem *item) {}

static BlinkFileItem *blink_file_item_new(GFile *file, GFileInfo *info) {
    BlinkFileItem *item = g_object_new(BLINK_TYPE_FILE_ITEM, NULL);
    item->file = g_object_ref(file);
    item->info = g_object_ref(info);
    item->path = g_file_get_path(file);
    return item;
}

static void apply_css(BlinkFiles *fm) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css,
        "window.blink-files {"
        "  background-color: #0d0d14;"
        "  color: #cdd6f4;"
        "}"
        "headerbar {"
        "  background-color: #0a0a0f;"
        "  border-bottom: 1px solid #1e1e2e;"
        "  color: #cdd6f4;"
        "}"
        "headerbar button {"
        "  background: none;"
        "  border: none;"
        "  border-radius: 6px;"
        "  color: #cdd6f4;"
        "  padding: 4px 8px;"
        "}"
        "headerbar button:hover { background-color: #1e1e2e; }"
        "entry.path-entry {"
        "  background-color: #1e1e2e;"
        "  border: 1px solid #313244;"
        "  border-radius: 8px;"
        "  color: #cdd6f4;"
        "  padding: 4px 12px;"
        "  font-size: 13px;"
        "}"
        ".sidebar {"
        "  background-color: #0a0a0f;"
        "  border-right: 1px solid #1e1e2e;"
        "  min-width: 180px;"
        "}"
        ".sidebar row {"
        "  border-radius: 6px;"
        "  margin: 2px 6px;"
        "  padding: 6px 8px;"
        "  color: #cdd6f4;"
        "}"
        ".sidebar row:selected {"
        "  background-color: #1e1e2e;"
        "}"
        ".file-grid {"
        "  background-color: #0d0d14;"
        "}"
        ".file-item {"
        "  border-radius: 8px;"
        "  padding: 8px;"
        "  color: #cdd6f4;"
        "}"
        ".file-item:hover { background-color: #1e1e2e; }"
        ".file-item label { font-size: 12px; }"
        "statusbar {"
        "  background-color: #0a0a0f;"
        "  border-top: 1px solid #1e1e2e;"
        "  color: #6c7086;"
        "  font-size: 12px;"
        "  padding: 2px 8px;"
        "}"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(css);
}

static void update_path_entry(BlinkFiles *fm) {
    gtk_editable_set_text(GTK_EDITABLE(fm->path_entry), fm->current_path);
    gtk_widget_set_sensitive(fm->back_btn, fm->history_pos > 0);
    gtk_widget_set_sensitive(fm->forward_btn,
        fm->history_pos < (gint)g_list_length(fm->history) - 1);
    gtk_widget_set_sensitive(fm->up_btn,
        g_strcmp0(fm->current_path, "/") != 0);
}

static GtkWidget *make_file_widget(BlinkFileItem *item) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class(box, "file-item");
    gtk_widget_set_size_request(box, 96, 96);

    GIcon *icon = g_file_info_get_icon(item->info);
    GtkWidget *img = gtk_image_new_from_gicon(icon);
    gtk_image_set_pixel_size(GTK_IMAGE(img), ICON_SIZE);
    gtk_box_append(GTK_BOX(box), img);

    const gchar *name = g_file_info_get_display_name(item->info);
    GtkWidget *label = gtk_label_new(name);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 12);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_lines(GTK_LABEL(label), 2);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    return box;
}

static void on_item_activated(GtkGridView *gv, guint pos, gpointer data) {
    BlinkFiles *fm = data;
    BlinkFileItem *item = g_list_model_get_item(G_LIST_MODEL(fm->store), pos);
    if (!item) return;

    GFileType ftype = g_file_info_get_file_type(item->info);
    if (ftype == G_FILE_TYPE_DIRECTORY) {
        blink_files_navigate(fm, item->path);
    } else {
        GError *err = NULL;
        g_app_info_launch_default_for_uri(
            g_file_get_uri(item->file), NULL, &err);
        if (err) { g_warning("Open failed: %s", err->message); g_error_free(err); }
    }
    g_object_unref(item);
}

static void sidebar_add_place(GtkListBox *lb, const gchar *label,
                               const gchar *icon_name, const gchar *path) {
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *icon = gtk_image_new_from_icon_name(icon_name);
    GtkWidget *lbl = gtk_label_new(label);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0);
    gtk_box_append(GTK_BOX(hbox), icon);
    gtk_box_append(GTK_BOX(hbox), lbl);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), hbox);
    g_object_set_data_full(G_OBJECT(row), "path", g_strdup(path), g_free);
    gtk_list_box_append(lb, row);
}

static void on_sidebar_row_activated(GtkListBox *lb, GtkListBoxRow *row, gpointer data) {
    BlinkFiles *fm = data;
    const gchar *path = g_object_get_data(G_OBJECT(row), "path");
    if (path) blink_files_navigate(fm, path);
}

static void on_back(GtkButton *btn, gpointer data) {
    BlinkFiles *fm = data;
    if (fm->history_pos <= 0) return;
    fm->history_pos--;
    g_free(fm->current_path);
    fm->current_path = g_strdup(g_list_nth_data(fm->history, fm->history_pos));
    blink_files_reload(fm);
    update_path_entry(fm);
}

static void on_forward(GtkButton *btn, gpointer data) {
    BlinkFiles *fm = data;
    if (fm->history_pos >= (gint)g_list_length(fm->history) - 1) return;
    fm->history_pos++;
    g_free(fm->current_path);
    fm->current_path = g_strdup(g_list_nth_data(fm->history, fm->history_pos));
    blink_files_reload(fm);
    update_path_entry(fm);
}

static void on_up(GtkButton *btn, gpointer data) {
    BlinkFiles *fm = data;
    GFile *f = g_file_new_for_path(fm->current_path);
    GFile *parent = g_file_get_parent(f);
    g_object_unref(f);
    if (!parent) return;
    gchar *path = g_file_get_path(parent);
    g_object_unref(parent);
    blink_files_navigate(fm, path);
    g_free(path);
}

static void on_path_activate(GtkEntry *entry, gpointer data) {
    BlinkFiles *fm = data;
    const gchar *path = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (g_file_test(path, G_FILE_TEST_IS_DIR))
        blink_files_navigate(fm, path);
}

static void setup_item(GtkSignalListItemFactory *f, GtkListItem *item, gpointer data) {
    GtkWidget *w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_list_item_set_child(item, w);
}

static void bind_item(GtkSignalListItemFactory *f, GtkListItem *item, gpointer data) {
    BlinkFileItem *fi = gtk_list_item_get_item(item);
    GtkWidget *box = gtk_list_item_get_child(item);
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(box)))
        gtk_box_remove(GTK_BOX(box), child);
    GtkWidget *widget = make_file_widget(fi);
    gtk_box_append(GTK_BOX(box), widget);
}

BlinkFiles *blink_files_new(GtkApplication *app) {
    BlinkFiles *fm = g_new0(BlinkFiles, 1);
    fm->app = app;
    fm->grid_mode = TRUE;
    fm->show_hidden = FALSE;
    fm->history_pos = -1;
    fm->store = g_list_store_new(BLINK_TYPE_FILE_ITEM);

    apply_css(fm);

    fm->window = gtk_application_window_new(app);
    gtk_widget_add_css_class(fm->window, "blink-files");
    gtk_window_set_title(GTK_WINDOW(fm->window), "Files");
    gtk_window_set_default_size(GTK_WINDOW(fm->window), 960, 600);

    fm->header = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(fm->window), fm->header);

    fm->back_btn = gtk_button_new_from_icon_name("go-previous-symbolic");
    fm->forward_btn = gtk_button_new_from_icon_name("go-next-symbolic");
    fm->up_btn = gtk_button_new_from_icon_name("go-up-symbolic");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(fm->header), fm->back_btn);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(fm->header), fm->forward_btn);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(fm->header), fm->up_btn);

    fm->path_entry = gtk_entry_new();
    gtk_widget_add_css_class(fm->path_entry, "path-entry");
    gtk_widget_set_hexpand(fm->path_entry, TRUE);
    gtk_widget_set_size_request(fm->path_entry, 400, -1);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(fm->header), fm->path_entry);

    GtkWidget *grid_btn = gtk_toggle_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(grid_btn), "view-grid-symbolic");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(fm->header), grid_btn);

    GtkWidget *hidden_btn = gtk_toggle_button_new();
    gtk_button_set_icon_name(GTK_BUTTON(hidden_btn), "view-more-symbolic");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(fm->header), hidden_btn);

    fm->paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(fm->paned), 180);

    fm->sidebar = gtk_list_box_new();
    gtk_widget_add_css_class(fm->sidebar, "sidebar");
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(fm->sidebar), GTK_SELECTION_SINGLE);

    const gchar *home = g_get_home_dir();
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Home",      "user-home-symbolic",     home);
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Desktop",   "user-desktop-symbolic",
        g_build_filename(home, "Desktop", NULL));
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Documents", "folder-documents-symbolic",
        g_build_filename(home, "Documents", NULL));
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Downloads", "folder-download-symbolic",
        g_build_filename(home, "Downloads", NULL));
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Music",     "folder-music-symbolic",
        g_build_filename(home, "Music", NULL));
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Pictures",  "folder-pictures-symbolic",
        g_build_filename(home, "Pictures", NULL));
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Videos",    "folder-videos-symbolic",
        g_build_filename(home, "Videos", NULL));
    sidebar_add_place(GTK_LIST_BOX(fm->sidebar), "Filesystem","drive-harddisk-symbolic", "/");

    GtkWidget *sidebar_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sidebar_scroll), fm->sidebar);
    gtk_widget_set_vexpand(sidebar_scroll, TRUE);
    gtk_paned_set_start_child(GTK_PANED(fm->paned), sidebar_scroll);

    GtkSignalListItemFactory *factory = GTK_SIGNAL_LIST_ITEM_FACTORY(gtk_signal_list_item_factory_new());
    g_signal_connect(factory, "setup", G_CALLBACK(setup_item), fm);
    g_signal_connect(factory, "bind",  G_CALLBACK(bind_item),  fm);

    GtkSelectionModel *sel = GTK_SELECTION_MODEL(gtk_multi_selection_new(G_LIST_MODEL(fm->store)));
    fm->grid_view = gtk_grid_view_new(sel, GTK_LIST_ITEM_FACTORY(factory));
    gtk_grid_view_set_min_columns(GTK_GRID_VIEW(fm->grid_view), 2);
    gtk_grid_view_set_max_columns(GTK_GRID_VIEW(fm->grid_view), 12);
    gtk_widget_add_css_class(fm->grid_view, "file-grid");

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), fm->grid_view);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_paned_set_end_child(GTK_PANED(fm->paned), scroll);

    fm->status_bar = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(fm->status_bar), 0);
    gtk_widget_add_css_class(fm->status_bar, "statusbar");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(vbox), fm->paned);
    gtk_box_append(GTK_BOX(vbox), fm->status_bar);
    gtk_widget_set_vexpand(fm->paned, TRUE);
    gtk_window_set_child(GTK_WINDOW(fm->window), vbox);

    g_signal_connect(fm->back_btn,    "clicked", G_CALLBACK(on_back), fm);
    g_signal_connect(fm->forward_btn, "clicked", G_CALLBACK(on_forward), fm);
    g_signal_connect(fm->up_btn,      "clicked", G_CALLBACK(on_up), fm);
    g_signal_connect(fm->path_entry,  "activate", G_CALLBACK(on_path_activate), fm);
    g_signal_connect(fm->sidebar, "row-activated", G_CALLBACK(on_sidebar_row_activated), fm);
    g_signal_connect(fm->grid_view, "activate", G_CALLBACK(on_item_activated), fm);

    return fm;
}

void blink_files_navigate(BlinkFiles *fm, const gchar *path) {
    if (!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) return;

    while ((gint)g_list_length(fm->history) - 1 > fm->history_pos) {
        GList *last = g_list_last(fm->history);
        g_free(last->data);
        fm->history = g_list_delete_link(fm->history, last);
    }

    fm->history = g_list_append(fm->history, g_strdup(path));
    fm->history_pos = g_list_length(fm->history) - 1;

    g_free(fm->current_path);
    fm->current_path = g_strdup(path);

    blink_files_reload(fm);
    update_path_entry(fm);
}

void blink_files_reload(BlinkFiles *fm) {
    g_list_store_remove_all(fm->store);

    GFile *dir = g_file_new_for_path(fm->current_path);
    GError *err = NULL;
    GFileEnumerator *en = g_file_enumerate_children(dir,
        "standard::*,thumbnail::*",
        G_FILE_QUERY_INFO_NONE, NULL, &err);

    if (err) {
        g_warning("Cannot open dir: %s", err->message);
        g_error_free(err);
        g_object_unref(dir);
        return;
    }

    GFileInfo *info;
    gint count = 0;
    while ((info = g_file_enumerator_next_file(en, NULL, NULL))) {
        const gchar *name = g_file_info_get_name(info);
        if (!fm->show_hidden && name[0] == '.') {
            g_object_unref(info);
            continue;
        }
        GFile *child = g_file_get_child(dir, name);
        BlinkFileItem *item = blink_file_item_new(child, info);
        g_list_store_append(fm->store, item);
        g_object_unref(item);
        g_object_unref(child);
        g_object_unref(info);
        count++;
    }

    g_object_unref(en);
    g_object_unref(dir);

    gchar *status = g_strdup_printf("  %d items", count);
    gtk_label_set_text(GTK_LABEL(fm->status_bar), status);
    g_free(status);
}

void blink_files_show(BlinkFiles *fm) {
    gtk_widget_set_visible(fm->window, TRUE);
}

void blink_files_free(BlinkFiles *fm) {
    g_list_free_full(fm->history, g_free);
    g_free(fm->current_path);
    g_object_unref(fm->store);
    g_free(fm);
}
