#!/bin/bash

while [[ -n "${1}" ]]; do
	case "${1}" in
		-d|--database)
			DATABASE="${2}"
			shift
			;;
		-U|--user)
			USER="${2}"
			shift
			;;
	esac
	shift
done

if [[ -z "${USER}" ]]; then
	echo "User was not specified" >&2
	exit 1
fi

if [[ -z "${DATABASE}" ]]; then
	echo "Database was not specified" >&2
	exit 1
fi

for scheme in deska test jsn api production history genproc versioning; do
    psql -d "${DATABASE}" -U "${USER}" -c "DROP SCHEMA ${scheme} CASCADE;"
done
