#pragma once

typedef struct _BlinkIPC BlinkIPC;
typedef struct _BlinkShell BlinkShell;

BlinkIPC *blink_ipc_new(BlinkShell *shell);
void      blink_ipc_start(BlinkIPC *ipc);
void      blink_ipc_free(BlinkIPC *ipc);
