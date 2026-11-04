#ifndef AUTOLOGIN_RECORD_H
#define AUTOLOGIN_RECORD_H

#include <pwd.h>
#include <sys/types.h>

#define WTMP "/var/log/wtmp"

/// wtmp file to append to, WTMP unless a test changes it
extern const char *record_wtmp;

/// Add the session to utmp and wtmp, so who and last show it
void record_login(const struct passwd *pw, pid_t pid);
/// Mark the session as ended in utmp and wtmp
void record_logout(pid_t pid);

#endif
