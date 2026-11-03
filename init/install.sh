#!/bin/sh
set -e

action=${1:-install}
SBINDIR=${SBINDIR:-/usr/local/sbin}
cd "$(dirname "$0")"

has() {
	command -v "$1" >/dev/null 2>&1
}

detect() {
	{ has openrc-run || [ -x /sbin/openrc-run ]; } && echo openrc
	{ has systemctl || [ -d /run/systemd/system ]; } && echo systemd
	has runsvdir && echo runit
	has dinit && echo dinit
	true
}

choose() {
	found=$(detect)
	count=$(echo "$found" | grep -c .)

	if [ "$count" -eq 0 ]; then
		echo "no supported init system found, set INIT=openrc|systemd|runit|dinit" >&2
		exit 1
	fi
	if [ "$count" -eq 1 ]; then
		echo "$found"
		return
	fi
	if ! true 2>/dev/null </dev/tty; then
		echo "found several init systems ($(echo $found)), set INIT= to pick one" >&2
		exit 1
	fi

	echo "Found several init systems:" >/dev/tty
	i=1
	for name in $found; do
		echo "  $i) $name" >/dev/tty
		i=$((i + 1))
	done
	printf "Which one? " >/dev/tty
	read -r pick </dev/tty
	picked=$(echo "$found" | sed -n "${pick}p")
	if [ -z "$picked" ]; then
		echo "invalid choice '$pick'" >&2
		exit 1
	fi
	echo "$picked"
}

runit_dir() {
	if [ -d "$DESTDIR/etc/runit/sv" ]; then
		echo "$DESTDIR/etc/runit/sv"
	else
		echo "$DESTDIR/etc/sv"
	fi
}

put() {
	mkdir -p "$(dirname "$3")"
	sed "s|@SBINDIR@|$SBINDIR|g" "$1" >"$3"
	chmod "$2" "$3"
	echo "installed $3"
}

install_init() {
	case $1 in
	openrc)
		put openrc/autologin 755 "$DESTDIR/etc/init.d/autologin"
		echo "enable with: rc-update add autologin default"
		;;
	systemd)
		put systemd/autologin.service 644 "$DESTDIR/etc/systemd/system/autologin.service"
		echo "enable with: systemctl daemon-reload && systemctl enable autologin"
		;;
	runit)
		dir=$(runit_dir)
		put runit/autologin/run 755 "$dir/autologin/run"
		echo "enable with: ln -s ${dir#"$DESTDIR"}/autologin /var/service (Void) or /run/runit/service (Artix)"
		;;
	dinit)
		put dinit/autologin 644 "$DESTDIR/etc/dinit.d/autologin"
		echo "enable with: dinitctl enable autologin"
		;;
	*)
		echo "unknown init system '$1', use openrc|systemd|runit|dinit" >&2
		exit 1
		;;
	esac
}

uninstall_init() {
	rm -f "$DESTDIR/etc/init.d/autologin" \
		"$DESTDIR/etc/systemd/system/autologin.service" \
		"$DESTDIR/etc/dinit.d/autologin"
	rm -rf "$DESTDIR/etc/sv/autologin" "$DESTDIR/etc/runit/sv/autologin"
}

case $action in
install)
	init=${INIT:-}
	[ -n "$init" ] || init=$(choose)
	install_init "$init"
	;;
uninstall) uninstall_init ;;
detect) detect ;;
*)
	echo "usage: $0 install|uninstall|detect" >&2
	exit 1
	;;
esac
