#!/bin/sh

SCRIPT_LOC="$(dirname "$(type "$0" | awk '{print $NF}')")"
cd "$SCRIPT_LOC"

make CMFLAGS="-DBUILD_TESTS=ON"

for test in $(find build/ -type f -executable | grep test); do
	if ! "./$test" -a; then
		exit $?
	fi
done
