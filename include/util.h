#ifndef AUTOLOGIN_UTIL_H
#define AUTOLOGIN_UTIL_H

/// Just error out
void error(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
/// Strip whitespace from both ends
char *trim(char *s);
/// Read a file into a string
char *read_file(const char *path);

#endif
