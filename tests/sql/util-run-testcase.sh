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

DESKA_CFGGEN_BACKEND=git
export DESKA_CFGGEN_BACKEND
DESKA_CFGGEN_GIT_REPO=${DESKA_GENERATED_FILES}/cfggen-repo
export DESKA_CFGGEN_GIT_REPO
DESKA_CFGGEN_GIT_PRIMARY_CLONE=${DESKA_GENERATED_FILES}/cfggen-primary
export DESKA_CFGGEN_GIT_PRIMARY_CLONE
DESKA_CFGGEN_GIT_WC=${DESKA_GENERATED_FILES}/cfggen-wc
export DESKA_CFGGEN_GIT_WC
DESKA_CFGGEN_SCRIPTS=${DESKA_GENERATED_FILES}/scripts
export DESKA_CFGGEN_SCRIPTS
DESKA_CFGGEN_GIT_SECOND=${DESKA_GENERATED_FILES}/second-wd
export DESKA_CFGGEN_GIT_SECOND

# Initialize the master repository which simulates a remote repo
git init --bare ${DESKA_CFGGEN_GIT_REPO} || die "git init --bare my_repo failed"
# Initialize the "primary clone"; that's the repository from which we create extra WCs
git clone ${DESKA_CFGGEN_GIT_REPO} ${DESKA_CFGGEN_GIT_PRIMARY_CLONE} || die "git clone my_repo my_primary_clone failed"
# Got to start the master branch now, or git-new-workdir will break
pushd ${DESKA_CFGGEN_GIT_PRIMARY_CLONE}
echo "This is a repo of the resulting configuration" > README
git add README || die "git add README failed"
git commit -m "Initial commit" || die "git commit failed"
git push origin master || die "git push origin master failed"
popd

# Prepare (empty) generating scripts
mkdir "${DESKA_CFGGEN_SCRIPTS}"

export DESKA_SOURCES

case "${TESTMODE}" in
    sql)
        if [[ ! -f $TESTCASE ]]; then
            die "Locating SQL testcase"
        fi

        pg_prove -U $DESKA_USER -d $DESKA_DB $TESTCASE || die "Test"
        ;;
    dbapi)
        export DESKA_USER
        export DESKA_DB
        python ${DESKA_SOURCES}/tests/dbapi-application/testdbapi.py ${DESKA_SOURCES}/src/deska/server/app/deska_server.py \
            $TESTCASE || die "Test"
        ;;
    persist)
        export DESKA_USER
        export DESKA_DB
        python ${DESKA_SOURCES}/tests/dbapi-persist/tester.py ${DESKA_SOURCES}/src/deska/server/app/deska_server.py \
            $TESTCASE || die "Test"
        ;;
    run)
        export DESKA_USER
        export DESKA_DB
        ${TESTCASE} || die "Test"
        ;;
    *)
        die "Unknown test"
esac

