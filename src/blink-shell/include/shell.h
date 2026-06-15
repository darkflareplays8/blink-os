#pragma once
#include "config.h"
#include "ipc.h"

typedef struct _BlinkShell BlinkShell;

BlinkShell   *blink_shell_new(void);
void          blink_shell_load_config(BlinkShell *s);
void          blink_shell_start_ipc(BlinkShell *s);
void          blink_shell_run(BlinkShell *s);
void          blink_shell_quit(BlinkShell *s);
void          blink_shell_free(BlinkShell *s);
BlinkConfig  *blink_shell_get_config(BlinkShell *s);
