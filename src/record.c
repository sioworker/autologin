#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <utmpx.h>

#include "record.h"
#include "session.h"

const char *record_wtmp = WTMP;

/// Copy src into a fixed size utmp field that doesn't need a terminating NUL
static void copy_field(char *dst, size_t size, const char *src) {
	size_t len = strlen(src);
	memset(dst, 0, size);
	memcpy(dst, src, len < size ? len : size);
}

/// Fill in the parts of an entry that login and logout share
static void fill(struct utmpx *ut, short type, pid_t pid) {
	char line[16];
	struct timeval tv;

	memset(ut, 0, sizeof *ut);
	snprintf(line, sizeof line, "tty%d", VT);
	size_t len = strlen(line);

	ut->ut_type = type;
	ut->ut_pid = pid;
	copy_field(ut->ut_line, sizeof ut->ut_line, line);
	copy_field(ut->ut_id, sizeof ut->ut_id, line + (len > sizeof ut->ut_id ? len - sizeof ut->ut_id : 0));

	gettimeofday(&tv, NULL);
	ut->ut_tv.tv_sec = tv.tv_sec;
	ut->ut_tv.tv_usec = tv.tv_usec;
}

/// Write the entry to utmp and append it to wtmp
static void write_entry(const struct utmpx *ut) {
	setutxent();
	pututxline(ut);
	endutxent();
	updwtmpx(record_wtmp, ut);
}

void record_login(const struct passwd *pw, pid_t pid) {
	struct utmpx ut;

	fill(&ut, USER_PROCESS, pid);
	copy_field(ut.ut_user, sizeof ut.ut_user, pw->pw_name);
	write_entry(&ut);
}

void record_logout(pid_t pid) {
	struct utmpx ut;

	fill(&ut, DEAD_PROCESS, pid);
	write_entry(&ut);
}
