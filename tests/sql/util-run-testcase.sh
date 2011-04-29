#/bin/bash

cd `dirname $0`

. ./util-config.sh

if [[ -z $1 ]]; then
    die "No test case to run. Execution"
fi

if [[ ! -f $1 ]]; then
    die "Locating SQL testcase"
fi

PG_CONFIG=`which pg_config` 2>/dev/null || die "Locating PG_CONFIG"
PGTAP_FILE=`$PG_CONFIG --sharedir`/contrib/pgtap.sql
if [[ ! -f $PGTAP_FILE ]]; then
    die "Locating the pgtap.sql"
fi

# Always remove stuff, so that a possible error in previous test does not leave
# stuff broken
./util-delete-database.sh 2>/dev/null

./util-create-database.sh || die "Preparing the DB environment"

psql -q -U $USER -d $DB -f $PGTAP_FILE || die "Installing pgTAP"

pushd ../../src/deska/server/db
./deska_install.sh install -u $SU -d $DB --all
popd

#psql -q -U $USER -d $DB -v ON_ERROR_STOP=1 -f $1 || die "SQL exection"
pg_prove -U $SU -d $DB $1 || die "Test"

./util-delete-database.sh || die "Removing the DB after the test"
