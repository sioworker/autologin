#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "util.h"

void parse_config(const char *path, struct config *cfg) {
	char *line = read_file(path);

	cfg->script = "";

	while (*line) {
		char *next = strchr(line, '\n');
		char *start = line;
		if (next) *next++ = '\0';
		else next = line + strlen(line);

		char *s = line;
		while (isspace((unsigned char)*s)) s++;
		if (*s == '\0' || *s == '#') {
		} else if (!strncmp(s, "user=", 5)) {
			char *val = s + 5;
			char *hash = strchr(val, '#');
			if (hash) *hash = '\0';
			snprintf(cfg->user, sizeof cfg->user, "%s", trim(val));
		} else {
			if (*next) next[-1] = '\n';
			cfg->script = start;
			break;
		}
		line = next;
	}

	if (!*cfg->user) error("%s: no user= set", path);
}
