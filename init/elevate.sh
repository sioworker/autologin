#!/bin/sh

writable() {
	dir=$(dirname "$1")
	while [ ! -e "$dir" ]; do
		dir=$(dirname "$dir")
	done
	[ -w "$dir" ] && { [ ! -e "$1" ] || [ -w "$1" ]; }
}

elevate() {
	SU=${SU:-}
	for path in "$@"; do
		writable "$path" && continue

		[ -n "$SU" ] && return
		if [ "$(id -u)" -eq 0 ]; then
			echo "cannot write $path even as root" >&2
			exit 1
		fi
		if command -v doas >/dev/null 2>&1; then
			SU=doas
		elif command -v sudo >/dev/null 2>&1; then
			SU=sudo
		else
			echo "cannot write $path and neither doas nor sudo was found, run as root" >&2
			exit 1
		fi
		echo "using $SU to write $path" >&2
		return
	done
	SU=
}

as_root() {
	if [ -n "$SU" ]; then
		"$SU" "$@"
	else
		"$@"
	fi
}
