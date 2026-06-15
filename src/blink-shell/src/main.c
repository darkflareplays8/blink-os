#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "../include/shell.h"
#include "../include/config.h"
#include "../include/ipc.h"

static BlinkShell *shell = NULL;

static void on_signal(int sig) {
    if (shell) blink_shell_quit(shell);
}

int main(int argc, char **argv) {
    gtk_init();

    signal(SIGTERM, on_signal);
    signal(SIGINT, on_signal);

    shell = blink_shell_new();
    blink_shell_load_config(shell);
    blink_shell_start_ipc(shell);
    blink_shell_run(shell);
    blink_shell_free(shell);

    return 0;
}
