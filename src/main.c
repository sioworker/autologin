#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "check.h"
#include "config.h"
#include "record.h"
#include "session.h"
#include "util.h"

static pid_t child;

/// Pass stop signals to the whole session
static void forward_signal(int sig) {
	if (child > 0) kill(-child, sig);
}

int main(int argc, char **argv) {
	const char *conf = DEFAULT_CONF;
	int check = 0;
	struct config cfg = {0};

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--check")) check = 1;
		else if (argv[i][0] == '-') error("unknown option '%s', usage: autologin [--check] [CONFIG]", argv[i]);
		else conf = argv[i];
	}

	if (check) return check_config(conf);
	if (geteuid() != 0) error("must be run as root");

	parse_config(conf, &cfg);

	struct passwd *pw = getpwnam(cfg.user);
	if (!pw) error("unknown user '%s'", cfg.user);

	if (!cfg.relogin && access(DONE, F_OK) == 0) run_getty();

	child = fork();
	if (child < 0) error("fork: %s", strerror(errno));
	if (child == 0) run_session(&cfg, pw);
	record_login(pw, child);

	struct sigaction sa = { .sa_handler = forward_signal };
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);

	int status;
	while (waitpid(child, &status, 0) < 0) 
		if (errno != EINTR) error("waitpid: %s", strerror(errno));

	kill(-child, SIGTERM);
	record_logout(child);
	reset_tty();

	if (!cfg.relogin) {
		int fd = open(DONE, O_WRONLY | O_CREAT, 0644);
		if (fd < 0) error("cannot create " DONE ": %s", strerror(errno));
		close(fd);
	}

	return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
