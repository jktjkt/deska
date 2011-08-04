#!/bin/bash

#set -x

DB_SOURCES=`readlink -f ../src/deska/server/db/`

if [[ -z "${DESKA_GENERATED_FILES}" ]]; then
    # do not pollute the source tree with generated files
    DESKA_GENERATED_FILES=`mktemp -d`
    trap "rm -rf $DESKA_GENERATED_FILES" EXIT
fi

function copy-update-file() {
    sed "s:import dutil:import sys\nsys.path.append('$DB_SOURCES')\nimport dutil:" \
        "${1}" | sed "s:-- create-all-modules-here:${EXPANDED_MODULES}:" > "${DESKA_GENERATED_FILES}/"`basename "${1}"`
}

function pylib(){
	cp "${DB_SOURCES}/$1" "${DESKA_GENERATED_FILES}/${1}"
}

function die(){
    echo "Die: $1" >&2
    exit 1
}

function help(){
	echo "deska_install.sh ACTION [options]
	ACTION: install | drop | clean | regenerate
options:
d(atabase): database name
u(ser): user
m(modules): run action for modules only (default)
a(ll): run action for whole deska"
}
function drop(){
	echo "Drop stage $1 ..."
	psql -d "$DATABASE" -U "$USER" -f "drop_${1}.sql" 2>&1 > /dev/null | grep -v "cascades"
}

function stage(){
	echo "Stage $1 ..."
	psql -d "$DATABASE" -U "$USER" -v ON_ERROR_STOP=1 -f "create_${1}.sql" -v dbname="$DATABASE" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function create_templates(){
	echo "Creating templates ..."
	psql -d "$DATABASE" -U "$USER" -v ON_ERROR_STOP=1 -f "templates.sql" -v dbname="$DATABASE" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function generate_merge(){
	echo "Generating templates ..."
	python "${DB_SOURCES}/gen_sql/merge_generator.py" "$DATABASE" "$USER" "${DESKA_GENERATED_FILES}/merge.sql"
}

function add_merge_relations(){
	echo "Generating merge relations ..."
	psql -d "$DATABASE" -U "$USER" -v ON_ERROR_STOP=1 -f "rel_merge.sql" -v dbname="$DATABASE" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function add_merge_link_triggers(){
	echo "Generating merge link triggers ..."
	psql -d "$DATABASE" -U "$USER" -v ON_ERROR_STOP=1 -f "trg_merge.sql" -v dbname="$DATABASE" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function generate(){
	echo "Generating stored procedures ..."
	python "${DB_SOURCES}/gen_sql/generator.py" "$DATABASE" "$USER" "${DESKA_GENERATED_FILES}/gen_schema.sql" > ${DB_SOURCES}/generated.py
}

function generate_templates(){
	echo "Generating templates ..."
	python "${DB_SOURCES}/gen_sql/template_generator.py" "$DATABASE" "$USER" "${DESKA_GENERATED_FILES}/templates.sql"
}


eval set -- getopt -o hma -l help modules all -n "deska_install.sh" -- "$@"

TYPE=M

while test -n "$1"
do
	case "$1" in
		install) 
			ACTION="I"
			;;
		restore)
			ACTION="R"
			;;
		regenerate)
			ACTION="F"
			;;
		drop)
			ACTION="D"
			;;
		-m|--modules)
			TYPE="M"
			;;
		-a|--all)
			TYPE="A"
			;;
		-d|--database)
			DATABASE="$2"
			shift
			;;
		-u|--user)
			USER="$2"
			shift
			;;
		-h|--help)
			ACTION="H"
			;;
	esac
	shift
done

DESKA_SCHEMA_PATH="${DB_SOURCES}/../../../../install/modules/${DESKA_SCHEME:-demo}/"
EXPANDED_MODULES=""
for x in "${DESKA_SCHEMA_PATH}"/*.sql; do
    EXPANDED_MODULES="${EXPANDED_MODULES}\\\\i ${x}\n"
done

# every time copy all source files needed into pwd
for FILE in "${DB_SOURCES}"/*.sql "${DB_SOURCES}/../../../../install"/*.sql; do
    copy-update-file "${FILE}"
done

cd "${DESKA_GENERATED_FILES}"

if test -z $ACTION
then
	echo "Action must be specified"
	help
	exit
fi
if test -z $USER
then
	echo "User must be specified"
	help
	exit
fi
if test -z $DATABASE
then
	echo "Database must be specified"
	help
	exit
fi
if test $ACTION == "H"
then
	help
	exit
fi

if test $ACTION == "D"
then
	echo "Drop DB $DATABASE"
	if test $TYPE == "A"
	then
		drop 0
		drop "tables"
	fi
	drop 1
	drop 2
fi

if test $ACTION == "I"
then
	echo "Install DB $DATABASE"
	if test $TYPE == "A"
	then
		stage "tables" || die "Error runnig stage tables"
		stage 0 || die "Error running stage 0"
		pylib dutil.py || die "Error installing python utils - are you root?"
	fi
	stage 1 || die "Error running stage 1"
	generate_merge || die "Error running generate merge relations"
	add_merge_relations || die "Error running add merge relations"
	generate_templates || die "Error running generate templates"
	create_templates || die "Error running creating templates"
	generate || die "Failed to generate stuff"
	stage "tables2" || die "Error running stage tables2"
	stage 2 || die "Error running stage 2"
	add_merge_link_triggers || die "Error running add merge link triggers"
fi

if test $ACTION == "F"
then
	echo "Regenerate functions in DB $DATABASE"
	drop 2
	drop 0
	pylib dutil.py || die "Error installing python utils - are you root?"
	stage 0 || die "Error running stage 0"
	generate || die "Failed to generate stuff"
	stage 2 || die "Error running stage 2"
fi

if test $ACTION == "R"
then
	echo "Restore DB $DATABASE"
	drop 2
	drop 1
	drop "tables"
	if test $TYPE == "A"
	then
		drop 0
	fi
	pylib dutil.py
	stage "tables" || die "Error running stage tables"
	if test $TYPE == "A"
	then
		stage 0 || die "Error running stage 0"
	fi
	stage 1 || die "Error running stage 1"
	generate_merge || die "Error running generate merge relations"
	add_merge_relations || die "Error running add merge relations"
	generate_templates || die "Error running generate templates"
	create_templates || die "Error running creating templates"
	generate || die "Failed to generate stuff"
	stage "tables2" || die "Error running stage tables2"
	stage 2 || die "Error running stage 2"
	add_merge_link_triggers || die "Error running add merge link triggers"
fi
