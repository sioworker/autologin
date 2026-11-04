#include <stdio.h>

#include "config.h"

/// Print what parse_config read from argv[1]
int main(int argc, char **argv) {
	struct config cfg = {0};

	if (argc != 2) {
		fprintf(stderr, "usage: %s CONFIG\n", argv[0]);
		return 2;
	}

	parse_config(argv[1], &cfg);
	printf("user=%s\nrelogin=%d\nscript:\n%s", cfg.user, cfg.relogin, cfg.script);
	return 0;
}
