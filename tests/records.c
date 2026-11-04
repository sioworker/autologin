#define _GNU_SOURCE
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmpx.h>

#include "record.h"

#define PID 424242

#ifdef __GLIBC__
/// Find an entry for PID with the given type in the file utmpxname points at
static int find(short type, const char *user) {
	struct utmpx *ut;
	int found = 0;

	setutxent();
	while ((ut = getutxent()))
		if (ut->ut_pid == PID && ut->ut_type == type && !strncmp(ut->ut_user, user, sizeof ut->ut_user) &&
		    !strncmp(ut->ut_line, "tty1", sizeof ut->ut_line))
			found++;
	endutxent();
	return found;
}
#endif

/// Login and logout must land in temporary utmp and wtmp files (glibc only, musl's utmp does nothing)
int main(int argc, char **argv) {
#ifndef __GLIBC__
	(void)argc;
	(void)argv;
	puts("skip records (this libc has no utmp)");
	return 0;
#else
	char utmp[4096], wtmp[4096];
	int ok = 1;

	if (argc != 2) {
		fprintf(stderr, "usage: %s DIR\n", argv[0]);
		return 2;
	}
	snprintf(utmp, sizeof utmp, "%s/test-utmp", argv[1]);
	snprintf(wtmp, sizeof wtmp, "%s/test-wtmp", argv[1]);
	unlink(utmp);
	unlink(wtmp);
	fclose(fopen(utmp, "w"));
	fclose(fopen(wtmp, "w"));

	struct passwd *pw = getpwuid(getuid());
	if (!pw) return 2;

	utmpxname(utmp);
	record_wtmp = wtmp;

	record_login(pw, PID);
	if (find(USER_PROCESS, pw->pw_name) != 1) ok = 0, puts("FAIL utmp has no login entry");
	record_logout(PID);
	if (find(USER_PROCESS, pw->pw_name) != 0) ok = 0, puts("FAIL utmp still has the login entry");
	if (find(DEAD_PROCESS, "") != 1) ok = 0, puts("FAIL utmp has no logout entry");

	utmpxname(wtmp);
	if (find(USER_PROCESS, pw->pw_name) != 1) ok = 0, puts("FAIL wtmp has no login entry");
	if (find(DEAD_PROCESS, "") != 1) ok = 0, puts("FAIL wtmp has no logout entry");

	if (ok) puts("ok   records");
	return ok ? 0 : 1;
#endif
}
