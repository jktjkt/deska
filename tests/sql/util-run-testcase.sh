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
if [[ -z `find . -atime -1 -name .db_installed` ]]; then
	./util-delete-database.sh 2>/dev/null
	./util-create-database.sh || die "Preparing the DB environment"

	pushd ../../src/deska/server/db
	./deska_install.sh install -u $DESKA_SU -d $DESKA_DB --all || die "Running deska_install"
	popd
	touch .db_installed
else
	# clean db is enough
	pushd ../../src/deska/server/db
	./deska_install.sh restore --all -u $DESKA_SU -d $DESKA_DB || die "Running deska_install"
	popd
fi

#psql -q -U $DESKA_USER -d $DESKA_DB -v ON_ERROR_STOP=1 -f $1 || die "SQL exection"
pg_prove -U $DESKA_SU -d $DESKA_DB $1 || die "Test"

