#!/bin/sh

SCRIPT_LOC="$(dirname "$(type "$0" | awk '{print $NF}')")"
cd "$SCRIPT_LOC" || exit

make CMFLAGS="-DBUILD_TESTS=ON"

for test in $(find build/ -type f -executable | grep test); do
	echo "[*] Running test $test"
	if ! "./$test" -a; then
		exit $?
	fi
done
