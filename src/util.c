#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	fputs("autologin: ", stderr);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(1);
}

char *trim(char *s) {
	while (isspace((unsigned char)*s)) s++;
	char *e = s + strlen(s);
	while (e > s && isspace((unsigned char)e[-1])) *--e = '\0';
	return s;
}

char *read_file(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) error("cannot open %s: %s", path, strerror(errno));

	size_t cap = 4096, len = 0, n;
	char *buf = malloc(cap);
	while (buf && (n = fread(buf + len, 1, cap - len - 1, f)) > 0) {
		len += n;
		if (cap - len - 1 == 0) buf = realloc(buf, cap *= 2);
	}
	if (!buf) error("out of memory");
	buf[len] = '\0';
	fclose(f);
	return buf;
}
