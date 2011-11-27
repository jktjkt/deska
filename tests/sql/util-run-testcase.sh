#!/bin/bash

cd `dirname $0`

if [[ x"$1" == "x--with-git" ]]; then
    export DESKA_WITH_GIT=1
    shift
fi

. ./util-config.sh

TESTMODE="${1}"
TESTCASE="${2}"

if [[ -z "${TESTMODE}" ]]; then
    die "SQL/DBAPI/whatever selection"
fi

if [[ -z $TESTCASE ]]; then
    die "No test case to run. Execution"
fi


if [[ -z ${DESKA_SKIP_DB_INIT} ]]; then
    DESKA_DB_STATE_FILE=./.db_initialized

    if [[ -n "${DESKA_SQL_TEST_FAST_BUT_NONDETERMINISTIC}" ]] && [[ -f $DESKA_DB_STATE_FILE ]]; then
        # Use the fast way of doing things
        pushd "${DESKA_SOURCES}/install"
        ./deska_install.sh restore --all -u $DESKA_SU -d $DESKA_DB || die "Running deska_install"
        popd
    else
        # Use the slower way which is completely deterministic

        # Always remove stuff, so that a possible error in previous test does not leave
        # stuff broken
        ./util-delete-database.sh 2>/dev/null

        ./util-create-database.sh || die "Preparing the DB environment"

        pushd "${DESKA_SOURCES}/install"
        ./deska_install.sh install -u $DESKA_SU -d $DESKA_DB --all || die "Running deska_install"
        popd
        touch $DESKA_DB_STATE_FILE
    fi
else
    echo "Skipping the DB init altogether"
fi

if [[ -z "${DESKA_GENERATED_FILES}" ]]; then
    # do not pollute the source tree with generated files
    DESKA_GENERATED_FILES=`mktemp -d`
    trap "rm -rf $DESKA_GENERATED_FILES" EXIT
fi

if [[ -n "${DESKA_WITH_GIT}" ]]; then
    . ./util-manage-git.sh || die "git stuff"
    deska_init_git "${DESKA_GENERATED_FILES}"
    export GIT_PYTHON_TRACE=full
fi

export DESKA_SOURCES

# clean the deska_server.log
> deska_server.log

case "${TESTMODE}" in
    sql)
        if [[ ! -f $TESTCASE ]]; then
            die "Locating SQL testcase"
        fi

        pg_prove -U $DESKA_USER -d $DESKA_DB $TESTCASE
        TEST_RESULT=$?
        ;;
    dbapi)
        export DESKA_USER
        export DESKA_DB
        python ${DESKA_SOURCES}/tests/dbapi-application/testdbapi.py ${DESKA_SOURCES}/src/deska/server/app/deska-server $TESTCASE
        TEST_RESULT=$?
        ;;
    persist)
        export DESKA_USER
        export DESKA_DB
        python ${DESKA_SOURCES}/tests/dbapi-persist/tester.py ${DESKA_SOURCES}/src/deska/server/app/deska-server $TESTCASE
        TEST_RESULT=$?
        ;;
    run)
        export DESKA_USER
        export DESKA_DB
        ${TESTCASE}
        TEST_RESULT=$?
        ;;
    *)
        die "Unknown test"
esac

if [[ ${TEST_RESULT} -ne 0 ]]; then
    cat deska_server.log
    die "Test failed"
fi
