#pragma once
#include <gtk/gtk.h>

typedef struct _BlinkFiles BlinkFiles;

BlinkFiles *blink_files_new(GtkApplication *app);
void        blink_files_navigate(BlinkFiles *fm, const gchar *path);
void        blink_files_reload(BlinkFiles *fm);
void        blink_files_show(BlinkFiles *fm);
void        blink_files_free(BlinkFiles *fm);
