#!/bin/sh
set -e

action=${1:-install}
SBINDIR=${SBINDIR:-/usr/local/sbin}
cd "$(dirname "$0")" || exit 1
. ./elevate.sh

BIN=$DESTDIR$SBINDIR/autologin
CONF=$DESTDIR/etc/autologin.conf

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
		echo "found several init systems ($(echo "$found" | tr '\n' ' ')), set INIT= to pick one" >&2
		exit 1
	fi

	echo "Found several init systems:" >/dev/tty
	echo "$found" | awk '{ print "  " NR ") " $0 }' >/dev/tty
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

service() {
	case $1 in
	openrc)
		src=openrc/autologin mode=755 dest=$DESTDIR/etc/init.d/autologin
		enable="rc-update add autologin default"
		;;
	systemd)
		src=systemd/autologin.service mode=644 dest=$DESTDIR/etc/systemd/system/autologin.service
		enable="systemctl daemon-reload && systemctl enable autologin"
		;;
	runit)
		dir=$(runit_dir)
		src=runit/autologin/run mode=755 dest=$dir/autologin/run
		enable="ln -s ${dir#"$DESTDIR"}/autologin /var/service (Void) or /run/runit/service (Artix)"
		;;
	dinit)
		src=dinit/autologin mode=644 dest=$DESTDIR/etc/dinit.d/autologin
		enable="dinitctl enable autologin"
		;;
	*)
		echo "unknown init system '$1', use openrc|systemd|runit|dinit" >&2
		exit 1
		;;
	esac
}

put() {
	as_root install -Dm"$2" "$1" "$3"
	echo "installed $3"
}

put_template() {
	tmp=$(mktemp)
	sed "s|@SBINDIR@|$SBINDIR|g" "$1" >"$tmp"
	put "$tmp" "$2" "$3"
	rm -f "$tmp"
}

install_all() {
	init=${INIT:-}
	[ -n "$init" ] || init=$(choose)
	service "$init"

	elevate "$BIN" "$dest" "$CONF"
	put ../autologin 755 "$BIN"
	put_template "$src" "$mode" "$dest"
	[ -e "$CONF" ] || put ../etc/autologin.conf 644 "$CONF"
	echo "enable with: $enable"
}

uninstall_all() {
	set -- "$BIN" \
		"$DESTDIR/etc/init.d/autologin" \
		"$DESTDIR/etc/systemd/system/autologin.service" \
		"$DESTDIR/etc/dinit.d/autologin" \
		"$DESTDIR/etc/sv/autologin" \
		"$DESTDIR/etc/runit/sv/autologin"
	elevate "$@"
	as_root rm -rf "$@"
}

case $action in
install) install_all ;;
uninstall) uninstall_all ;;
detect) detect ;;
*)
	echo "usage: $0 install|uninstall|detect" >&2
	exit 1
	;;
esac
