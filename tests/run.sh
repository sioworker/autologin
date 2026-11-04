#!/bin/sh

cd "$(dirname "$0")/.." || exit 1
parse=${1:-build/parse}
failed=0

for conf in tests/configs/*.conf; do
	expected=${conf%.conf}.out
	actual=$("$parse" "$conf" 2>&1; echo "exit=$?")

	if [ "$actual" = "$(cat "$expected")" ]; then
		echo "ok   $conf"
	else
		echo "FAIL $conf"
		printf '%s\n' "$actual" | diff "$expected" - || true
		failed=1
	fi
done

exit "$failed"
