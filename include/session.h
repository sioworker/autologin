#ifndef AUTOLOGIN_SESSION_H
#define AUTOLOGIN_SESSION_H

#include <pwd.h>

#include "config.h"

// You might want to change this! there's a possibility that your terminal will exec this as its shell
#define SHELL "/bin/sh"
#define VT    1
#define GETTY "/sbin/getty"
#define DONE  "/run/autologin.done"

/// Log the user in on VT and run the script with SHELL
void run_session(const struct config *cfg, const struct passwd *pw) __attribute__((noreturn));
/// Give the tty back to root
void reset_tty(void);
/// Replace ourselves with a normal login prompt on VT
void run_getty(void) __attribute__((noreturn));

#endif
