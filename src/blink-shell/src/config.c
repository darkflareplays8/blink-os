#include <glib.h>
#include <string.h>
#include "../include/config.h"

struct _BlinkConfig {
    GKeyFile *kf;
    gchar    *path;
};

BlinkConfig *blink_config_load(void) {
    BlinkConfig *c = g_new0(BlinkConfig, 1);
    c->kf = g_key_file_new();

    const gchar *home = g_get_home_dir();
    c->path = g_build_filename(home, ".config", "blink", "shell.conf", NULL);

    GError *err = NULL;
    if (!g_key_file_load_from_file(c->kf, c->path, G_KEY_FILE_KEEP_COMMENTS, &err)) {
        g_clear_error(&err);
        blink_config_set_defaults(c);
        blink_config_save(c);
    }

    return c;
}

void blink_config_set_defaults(BlinkConfig *c) {
    g_key_file_set_string(c->kf, "shell", "window_manager", "openbox");
    g_key_file_set_string(c->kf, "shell", "wallpaper", "/usr/share/blink/wallpaper/default.png");
    g_key_file_set_string(c->kf, "shell", "theme", "blink-dark");
    g_key_file_set_string(c->kf, "shell", "accent_color", "#00d4ff");
    g_key_file_set_string(c->kf, "shell", "font", "Inter 10");
    g_key_file_set_boolean(c->kf, "shell", "animations", TRUE);
    g_key_file_set_integer(c->kf, "shell", "animation_speed_ms", 180);
}

void blink_config_save(BlinkConfig *c) {
    gchar *dir = g_path_get_dirname(c->path);
    g_mkdir_with_parents(dir, 0755);
    g_free(dir);

    GError *err = NULL;
    g_key_file_save_to_file(c->kf, c->path, &err);
    if (err) {
        g_warning("Config save failed: %s", err->message);
        g_error_free(err);
    }
}

const gchar *blink_config_get_string(BlinkConfig *c, const gchar *group, const gchar *key, const gchar *fallback) {
    GError *err = NULL;
    gchar *val = g_key_file_get_string(c->kf, group, key, &err);
    if (err) {
        g_clear_error(&err);
        return fallback;
    }
    return val;
}

gboolean blink_config_get_bool(BlinkConfig *c, const gchar *group, const gchar *key, gboolean fallback) {
    GError *err = NULL;
    gboolean val = g_key_file_get_boolean(c->kf, group, key, &err);
    if (err) { g_clear_error(&err); return fallback; }
    return val;
}

gint blink_config_get_int(BlinkConfig *c, const gchar *group, const gchar *key, gint fallback) {
    GError *err = NULL;
    gint val = g_key_file_get_integer(c->kf, group, key, &err);
    if (err) { g_clear_error(&err); return fallback; }
    return val;
}

void blink_config_set_string(BlinkConfig *c, const gchar *group, const gchar *key, const gchar *val) {
    g_key_file_set_string(c->kf, group, key, val);
    blink_config_save(c);
}

void blink_config_free(BlinkConfig *c) {
    g_key_file_free(c->kf);
    g_free(c->path);
    g_free(c);
}
