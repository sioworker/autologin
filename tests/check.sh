#!/bin/sh

cd "$(dirname "$0")/.." || exit 1
autologin=${1:-./autologin}
failed=0

for conf in tests/check/*.conf; do
	case $(basename "$conf") in
	fail-*) want=1 ;;
	*) want=0 ;;
	esac

	output=$("$autologin" --check "$conf" 2>&1)
	got=$?

	if [ "$got" -eq "$want" ]; then
		echo "ok   $conf"
	else
		echo "FAIL $conf (exit $got, wanted $want)"
		printf '%s\n' "$output"
		failed=1
	fi
done

exit "$failed"
