#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "../include/shell.h"
#include "../include/config.h"
#include "../include/ipc.h"

struct _BlinkShell {
    GMainLoop     *loop;
    BlinkConfig   *config;
    BlinkIPC      *ipc;
    GList         *panels;
    GPid           wm_pid;
    gboolean       running;
};

BlinkShell *blink_shell_new(void) {
    BlinkShell *s = g_new0(BlinkShell, 1);
    s->loop = g_main_loop_new(NULL, FALSE);
    s->running = TRUE;
    return s;
}

void blink_shell_load_config(BlinkShell *s) {
    s->config = blink_config_load();
}

void blink_shell_start_ipc(BlinkShell *s) {
    s->ipc = blink_ipc_new(s);
    blink_ipc_start(s->ipc);
}

static void spawn_wm(BlinkShell *s) {
    const gchar *wm = blink_config_get_string(s->config, "shell", "window_manager", "openbox");
    gchar *argv[] = { (gchar *)wm, NULL };
    GError *err = NULL;
    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &s->wm_pid, &err);
    if (err) {
        g_warning("Failed to spawn WM: %s", err->message);
        g_error_free(err);
    }
}

static void spawn_panel(BlinkShell *s) {
    gchar *argv[] = { "blink-panel", NULL };
    GError *err = NULL;
    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &err);
    if (err) {
        g_warning("Failed to spawn panel: %s", err->message);
        g_error_free(err);
    }
}

static void spawn_notif(BlinkShell *s) {
    gchar *argv[] = { "blink-notif", NULL };
    GError *err = NULL;
    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &err);
    if (err) {
        g_warning("Failed to spawn notif: %s", err->message);
        g_error_free(err);
    }
}

static void set_wallpaper(BlinkShell *s) {
    const gchar *wp = blink_config_get_string(s->config, "shell", "wallpaper",
        "/usr/share/blink/wallpaper/default.png");
    gchar *cmd = g_strdup_printf("feh --bg-scale \"%s\"", wp);
    system(cmd);
    g_free(cmd);
}

void blink_shell_run(BlinkShell *s) {
    spawn_wm(s);
    g_usleep(500000);
    set_wallpaper(s);
    spawn_panel(s);
    spawn_notif(s);
    g_main_loop_run(s->loop);
}

void blink_shell_quit(BlinkShell *s) {
    if (s->wm_pid) kill(s->wm_pid, SIGTERM);
    g_main_loop_quit(s->loop);
}

void blink_shell_free(BlinkShell *s) {
    if (s->ipc) blink_ipc_free(s->ipc);
    if (s->config) blink_config_free(s->config);
    g_main_loop_unref(s->loop);
    g_free(s);
}

BlinkConfig *blink_shell_get_config(BlinkShell *s) {
    return s->config;
}
