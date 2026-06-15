#pragma once
#include <gtk/gtk.h>
#include <glib.h>

typedef struct _BlinkPanel BlinkPanel;

BlinkPanel *blink_panel_new(void);
void        blink_panel_load_config(BlinkPanel *p);
void        blink_panel_load_plugins(BlinkPanel *p);
void        blink_panel_show(BlinkPanel *p);
void        blink_panel_free(BlinkPanel *p);
