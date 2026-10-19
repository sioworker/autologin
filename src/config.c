#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "util.h"

/// Value after the key, without a trailing # comment
static char *value(char *s) {
	char *hash = strchr(s, '#');
	if (hash) *hash = '\0';
	return trim(s);
}

void parse_config(const char *path, struct config *cfg) {
	char *line = read_file(path);

	cfg->script = NULL;
	cfg->relogin = 1;

	while (*line) {
		char *next = strchr(line, '\n');
		if (next) *next++ = '\0';
		else next = line + strlen(line);

		char *s = line;
		while (isspace((unsigned char)*s)) s++;
		if (*s == '\0' || *s == '#') {
		} else if (!strncmp(s, "user=", 5)) {
			snprintf(cfg->user, sizeof cfg->user, "%s", value(s + 5));
		} else if (!strncmp(s, "relogin=", 8)) {
			char *val = value(s + 8);
			if (!strcmp(val, "yes")) cfg->relogin = 1;
			else if (!strcmp(val, "no")) cfg->relogin = 0;
			else error("%s: relogin= must be yes or no", path);
		} else if (!strncmp(s, "script=", 7)) {
			cfg->script = next;
			break;
		} else {
			error("%s: unknown line '%s'", path, s);
		}
		line = next;
	}

	if (!*cfg->user) error("%s: no user= set", path);
	if (!cfg->script) error("%s: no script= set", path);
}
