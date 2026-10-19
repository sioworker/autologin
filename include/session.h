#ifndef AUTOLOGIN_SESSION_H
#define AUTOLOGIN_SESSION_H

#include <pwd.h>

#include "config.h"

#define SHELL "/bin/sh"
#define VT    1

/// Log the user in on VT and run the script with SHELL
void run_session(const struct config *cfg, const struct passwd *pw) __attribute__((noreturn));
/// Give the tty back to root
void reset_tty(void);

#endif
