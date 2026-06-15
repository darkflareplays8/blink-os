#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>
#include <glib.h>
#include <string.h>
#include "../include/ipc.h"
#include "../include/shell.h"

#define BLINK_IPC_SOCKET "/tmp/blink-shell.sock"

struct _BlinkIPC {
    BlinkShell      *shell;
    GSocketService  *service;
    gchar           *socket_path;
};

static gboolean on_incoming(GSocketService *svc, GSocketConnection *conn,
                             GObject *src, gpointer data) {
    BlinkIPC *ipc = data;
    GInputStream *in = g_io_stream_get_input_stream(G_IO_STREAM(conn));
    GOutputStream *out = g_io_stream_get_output_stream(G_IO_STREAM(conn));

    gchar buf[512] = {0};
    gsize n = 0;
    GError *err = NULL;

    g_input_stream_read(in, buf, sizeof(buf) - 1, NULL, &err);
    if (err) { g_error_free(err); return FALSE; }

    gchar *resp = NULL;

    if (g_str_has_prefix(buf, "quit")) {
        blink_shell_quit(ipc->shell);
        resp = g_strdup("ok\n");
    } else if (g_str_has_prefix(buf, "reload-config")) {
        blink_shell_load_config(ipc->shell);
        resp = g_strdup("ok\n");
    } else if (g_str_has_prefix(buf, "set-wallpaper ")) {
        const gchar *wp = buf + 14;
        blink_config_set_string(blink_shell_get_config(ipc->shell), "shell", "wallpaper", wp);
        gchar *cmd = g_strdup_printf("feh --bg-scale \"%s\"", wp);
        system(cmd);
        g_free(cmd);
        resp = g_strdup("ok\n");
    } else if (g_str_has_prefix(buf, "set-theme ")) {
        const gchar *theme = buf + 10;
        blink_config_set_string(blink_shell_get_config(ipc->shell), "shell", "theme", theme);
        resp = g_strdup("ok\n");
    } else {
        resp = g_strdup("unknown-command\n");
    }

    g_output_stream_write(out, resp, strlen(resp), NULL, NULL);
    g_free(resp);
    return FALSE;
}

BlinkIPC *blink_ipc_new(BlinkShell *shell) {
    BlinkIPC *ipc = g_new0(BlinkIPC, 1);
    ipc->shell = shell;
    ipc->socket_path = g_strdup(BLINK_IPC_SOCKET);
    return ipc;
}

void blink_ipc_start(BlinkIPC *ipc) {
    g_unlink(ipc->socket_path);

    GError *err = NULL;
    ipc->service = g_socket_service_new();

    GSocketAddress *addr = g_unix_socket_address_new(ipc->socket_path);
    g_socket_listener_add_address(G_SOCKET_LISTENER(ipc->service), addr,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL, NULL, &err);
    g_object_unref(addr);

    if (err) {
        g_warning("IPC failed: %s", err->message);
        g_error_free(err);
        return;
    }

    g_signal_connect(ipc->service, "incoming", G_CALLBACK(on_incoming), ipc);
    g_socket_service_start(ipc->service);
}

void blink_ipc_free(BlinkIPC *ipc) {
    if (ipc->service) {
        g_socket_service_stop(ipc->service);
        g_object_unref(ipc->service);
    }
    g_unlink(ipc->socket_path);
    g_free(ipc->socket_path);
    g_free(ipc);
}
