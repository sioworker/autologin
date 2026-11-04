#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "check.h"
#include "config.h"
#include "session.h"

/// Run SHELL -n on the script, it prints its own syntax errors
static int script_syntax_ok(const char *script) {
	fflush(stdout);
	pid_t pid = fork();
	if (pid < 0) return 0;
	if (pid == 0) {
		execl(SHELL, "sh", "-n", "-c", script, (char *)NULL);
		_exit(127);
	}

	int status;
	if (waitpid(pid, &status, 0) < 0) return 0;
	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/// 1 if the user is in the group, 0 if not, -1 if the group doesn't exist
static int in_group(const struct passwd *pw, const char *name) {
	struct group *gr = getgrnam(name);
	if (!gr) return -1;
	if (gr->gr_gid == pw->pw_gid) return 1;
	for (char **m = gr->gr_mem; *m; m++)
		if (!strcmp(*m, pw->pw_name)) return 1;
	return 0;
}

/// Print the script indented, one line at a time
static void print_script(const char *script) {
	while (*script) {
		const char *end = strchr(script, '\n');
		int len = end ? (int)(end - script) : (int)strlen(script);
		printf("  %.*s\n", len, script);
		script += len + (end ? 1 : 0);
	}
}

int check_config(const char *path) {
	struct config cfg = {0};
	int problems = 0;
	struct stat st;

	parse_config(path, &cfg);
	printf("config:  %s\n", path);

	struct passwd *pw = getpwnam(cfg.user);
	if (!pw) {
		printf("user:    %s\nerror:   user '%s' does not exist\n", cfg.user, cfg.user);
		problems++;
	} else {
		printf("user:    %s (uid %u, home %s)\n", pw->pw_name, (unsigned)pw->pw_uid, pw->pw_dir);
		if (stat(pw->pw_dir, &st) < 0 || !S_ISDIR(st.st_mode))
			printf("warning: home %s does not exist, the script will start in /\n", pw->pw_dir);
		if (in_group(pw, "seat") == 0)
			printf("warning: %s is not in the seat group, seatd will refuse it\n", pw->pw_name);
	}

	printf("relogin: %s\n", cfg.relogin ? "yes" : "no");

	if (strspn(cfg.script, " \t\r\n") == strlen(cfg.script)) {
		printf("script:  (empty, an interactive " SHELL " will start)\n");
	} else {
		printf("script:\n");
		print_script(cfg.script);
		if (!script_syntax_ok(cfg.script)) {
			printf("error:   the script has a syntax error\n");
			problems++;
		}
	}

	printf("%s\n", problems ? "not ok" : "ok");
	return problems ? 1 : 0;
}
