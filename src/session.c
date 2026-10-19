#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <linux/vt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "session.h"
#include "util.h"

/// Device path of VT
static const char *tty_path(void) {
	static char path[32];
	snprintf(path, sizeof path, "/dev/tty%d", VT);
	return path;
}

/// Make VT the controlling tty and switch to it
static void setup_tty(const struct passwd *pw) {
	const char *tty = tty_path();

	if (setsid() < 0) error("setsid: %s", strerror(errno));

	int fd = open(tty, O_RDWR | O_NOCTTY);
	if (fd < 0) error("cannot open %s: %s", tty, strerror(errno));
	if (ioctl(fd, TIOCSCTTY, 1) < 0) error("TIOCSCTTY on %s: %s", tty, strerror(errno));

	struct group *tty_grp = getgrnam("tty");
	if (fchown(fd, pw->pw_uid, tty_grp ? tty_grp->gr_gid : pw->pw_gid) < 0 || fchmod(fd, 0620) < 0)
		error("cannot chown %s: %s", tty, strerror(errno));

	if (ioctl(fd, VT_ACTIVATE, VT) < 0 || ioctl(fd, VT_WAITACTIVE, VT) < 0)
		fprintf(stderr, "autologin: cannot switch to %s: %s\n", tty, strerror(errno));

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd > STDERR_FILENO) close(fd);
}

/// Create /run/user/<uid>
static void setup_runtime_dir(const struct passwd *pw, char *out, size_t len) {
	snprintf(out, len, "/run/user/%u", (unsigned)pw->pw_uid);
	mkdir("/run/user", 0755);
	if (mkdir(out, 0700) < 0 && errno != EEXIST) error("mkdir %s: %s", out, strerror(errno));
	if (chown(out, pw->pw_uid, pw->pw_gid) < 0 || chmod(out, 0700) < 0) error("cannot chown %s: %s", out, strerror(errno));
}

/// Become the user
static void drop_privileges(const struct passwd *pw) {
	if (initgroups(pw->pw_name, pw->pw_gid) < 0 || setgid(pw->pw_gid) < 0 || setuid(pw->pw_uid) < 0)
		error("cannot drop privileges: %s", strerror(errno));
	if (pw->pw_uid != 0 && setuid(0) == 0) error("privileges were not dropped");
}

/// Clean login environment
static void setup_env(const struct passwd *pw, const char *runtime_dir) {
	char vtnr[8];
	snprintf(vtnr, sizeof vtnr, "%d", VT);

	clearenv();
	setenv("HOME", pw->pw_dir, 1);
	setenv("USER", pw->pw_name, 1);
	setenv("LOGNAME", pw->pw_name, 1);
	setenv("SHELL", SHELL, 1);
	setenv("PATH", "/usr/local/bin:/usr/bin:/bin", 1);
	setenv("TERM", "linux", 1);
	setenv("XDG_RUNTIME_DIR", runtime_dir, 1);
	setenv("XDG_SEAT", "seat0", 1);
	setenv("XDG_VTNR", vtnr, 1);
}

void run_session(const struct config *cfg, const struct passwd *pw) {
	char runtime_dir[64];

	setup_runtime_dir(pw, runtime_dir, sizeof runtime_dir);
	setup_tty(pw);
	drop_privileges(pw);

	if (chdir(pw->pw_dir) < 0 && chdir("/") < 0)
		error("chdir: %s", strerror(errno));

	setup_env(pw, runtime_dir);

	if (*trim(cfg->script))
		execl(SHELL, "-sh", "-c", cfg->script, (char *)NULL);
	else
		execl(SHELL, "-sh", (char *)NULL);
	error("exec " SHELL ": %s", strerror(errno));
}

void reset_tty(void) {
	const char *tty = tty_path();

	if (chown(tty, 0, 0) < 0 || chmod(tty, 0600) < 0)
		fprintf(stderr, "autologin: cannot reset %s: %s\n", tty, strerror(errno));
}

void run_getty(void) {
	const char *tty = tty_path();

	execl(GETTY, "getty", "38400", tty + strlen("/dev/"), (char *)NULL);
	error("exec " GETTY ": %s", strerror(errno));
}
