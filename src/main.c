#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "session.h"
#include "util.h"

static pid_t child;

/// Pass stop signals to the whole session
static void forward_signal(int sig) {
	if (child > 0) kill(-child, sig);
}

int main(int argc, char **argv) {
	const char *conf = argc > 1 ? argv[1] : DEFAULT_CONF;
	struct config cfg = {0};

	if (geteuid() != 0) error("must be run as root");

	parse_config(conf, &cfg);

	struct passwd *pw = getpwnam(cfg.user);
	if (!pw) error("unknown user '%s'", cfg.user);

	child = fork();
	if (child < 0) error("fork: %s", strerror(errno));
	if (child == 0) run_session(&cfg, pw);

	struct sigaction sa = { .sa_handler = forward_signal };
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);

	int status;
	while (waitpid(child, &status, 0) < 0) 
		if (errno != EINTR) error("waitpid: %s", strerror(errno));

	kill(-child, SIGTERM);
	reset_tty();

	return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
