#/bin/bash

cd `dirname $0`

. ./util-config.sh

if [[ -z $1 ]]; then
    die "No test case to run. Execution"
fi

if [[ ! -f $1 ]]; then
    die "Locating SQL testcase"
fi

# Always remove stuff, so that a possible error in previous test does not leave
# stuff broken
./util-delete-database.sh 2>/dev/null

./util-create-database.sh || die "Cannot prepare the DB environment"

# FIXME: setup deska here

psql -q -U $USER -d $DB -v ON_ERROR_STOP=1 -f $1 || die "SQL error"

./util-delete-database.sh || die "Cannot remove the DB after the test"
