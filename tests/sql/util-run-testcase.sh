#!/bin/bash

cd `dirname $0`

. ./util-config.sh

TESTMODE="${1}"
TESTCASE="${2}"

if [[ -z "${TESTMODE}" ]]; then
    die "SQL/DBAPI/whatever selection"
fi

if [[ -z $TESTCASE ]]; then
    die "No test case to run. Execution"
fi

DESKA_DB_STATE_FILE=./.db_initialized

if [[ -n "${DESKA_SQL_TEST_FAST_BUT_NONDETERMINISTIC}" ]] && [[ -f $DESKA_DB_STATE_FILE ]]; then
    # Use the fast way of doing things
	pushd "${DESKA_SOURCES}/src/deska/server/db"
	./deska_install.sh restore --all -u $DESKA_SU -d $DESKA_DB || die "Running deska_install"
	popd
else
    # Use the slower way which is completely deterministic

    # Always remove stuff, so that a possible error in previous test does not leave
    # stuff broken
	./util-delete-database.sh 2>/dev/null

	./util-create-database.sh || die "Preparing the DB environment"

	pushd "${DESKA_SOURCES}/src/deska/server/db"
	./deska_install.sh install -u $DESKA_SU -d $DESKA_DB --all || die "Running deska_install"
	popd
	touch $DESKA_DB_STATE_FILE
fi

case "${TESTMODE}" in
    sql)
        if [[ ! -f $TESTCASE ]]; then
            die "Locating SQL testcase"
        fi

        pg_prove -U $DESKA_USER -d $DESKA_DB $TESTCASE || die "Test"
        ;;
    *)
        die "Unknown test"
esac

