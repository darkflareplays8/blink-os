#pragma once
#include <glib.h>

typedef struct _BlinkConfig BlinkConfig;

BlinkConfig  *blink_config_load(void);
void          blink_config_set_defaults(BlinkConfig *c);
void          blink_config_save(BlinkConfig *c);
void          blink_config_free(BlinkConfig *c);

const gchar  *blink_config_get_string(BlinkConfig *c, const gchar *group, const gchar *key, const gchar *fallback);
gboolean      blink_config_get_bool(BlinkConfig *c, const gchar *group, const gchar *key, gboolean fallback);
gint          blink_config_get_int(BlinkConfig *c, const gchar *group, const gchar *key, gint fallback);
void          blink_config_set_string(BlinkConfig *c, const gchar *group, const gchar *key, const gchar *val);
