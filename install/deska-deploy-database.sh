#!/bin/bash

#set -x

DB_SOURCES=`readlink -f ../src/deska/server/db/`

if [[ -z "${DESKA_GENERATED_FILES}" ]]; then
    # do not pollute the source tree with generated files
    DESKA_GENERATED_FILES=`mktemp -d`
    trap "rm -rf ${DESKA_GENERATED_FILES}" EXIT
fi

function copy-update-file() {
    sed "s:import dutil:import sys\nsys.path.append('${TARGET}/python')\nimport dutil:" \
        "${1}" | sed "s:-- create-all-modules-here:${EXPANDED_MODULES}:" > "${DESKA_GENERATED_FILES}/"`basename "${1}"`
}

function die(){
    echo "Die: ${1}" >&2
    exit 1
}

function help() {
    echo "Usage: ${0} -U user -d dbname -t path/to/put/server-side-files"
}

function stage(){
	echo "Stage $1 ..."
	psql -d "${DATABASE}" -U "${USER}" -v ON_ERROR_STOP=1 -f "create_${1}.sql" -v dbname="${DATABASE}" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function create_templates(){
	echo "Creating templates ..."
	psql -d "${DATABASE}" -U "${USER}" -v ON_ERROR_STOP=1 -f "templates.sql" -v dbname="${DATABASE}" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function generate_merge(){
	echo "Generating templates ..."
	python "${DB_SOURCES}/gen_sql/merge_generator.py" "${DATABASE}" "${USER}" "${DESKA_GENERATED_FILES}/merge.sql"
}

function add_merge_relations(){
	echo "Generating merge relations ..."
	psql -d "${DATABASE}" -U "${USER}" -v ON_ERROR_STOP=1 -f "rel_merge.sql" -v dbname="${DATABASE}" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function add_merge_link_triggers(){
	echo "Generating merge link triggers ..."
	psql -d "${DATABASE}" -U "${USER}" -v ON_ERROR_STOP=1 -f "trg_merge.sql" -v dbname="${DATABASE}" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function generate(){
	echo "Generating stored procedures ..."
	python "${DB_SOURCES}/gen_sql/generator.py" "${DATABASE}" "${USER}" "${DESKA_GENERATED_FILES}/gen_schema.sql" > ${TARGET}/python/generated.py
}

function generate_templates(){
	echo "Generating templates ..."
	python "${DB_SOURCES}/gen_sql/template_generator.py" "${DATABASE}" "${USER}" "${DESKA_GENERATED_FILES}/templates.sql"
}

function generate_multiRefs(){
	echo "Generating multi references ..."
	python "${DB_SOURCES}/gen_sql/multiRef_generator.py" "${DATABASE}" "${USER}" "${DESKA_GENERATED_FILES}/multiref.sql"
}

function add_multiRefs(){
	echo "Creating multi references ..."
	psql -d "${DATABASE}" -U "${USER}" -v ON_ERROR_STOP=1 -f "tab_multiref.sql" -v dbname="${DATABASE}" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

function add_multiRefs_functions(){
	echo "Adding multi references functions..."
	psql -d "${DATABASE}" -U "${USER}" -v ON_ERROR_STOP=1 -f "fn_multiref.sql" -v dbname="${DATABASE}" 2>&1 > /dev/null \
		|| return $? \
		| grep -v NOTICE | grep -v "current transaction is aborted"
}

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
        -t|--target)
            TARGET="${2}"
            shift
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

if [[ -z "${USER}" ]]; then
	echo "User was not specified" >&2
	help
	exit 1
fi

if [[ -z "${DATABASE}" ]]; then
	echo "Database was not specified" >&2
	help
	exit 1
fi

if [[ -z "${TARGET}" ]]; then
    echo "Target was not specified" >&2
    help
    exit 1
fi

mkdir "${TARGET}" || die "Directory ${TARGET} already exists"
TARGET=$(cd "${TARGET}"; pwd)
mkdir "${TARGET}/python" || die "Cannot mkdir ${TARGET}/python"

echo "Install DB ${DATABASE}"
stage "tables" || die "Error runnig stage tables"
stage 0 || die "Error running stage 0"
cp "${DB_SOURCES}/"*.py "${TARGET}/python/" || die "Error installing server-side Python files"
stage 1 || die "Error running stage 1"
generate_merge || die "Error running generate merge relations"
add_merge_relations || die "Error running add merge relations"
generate_templates || die "Error running generate templates"
create_templates || die "Error running creating templates"
generate_multiRefs || die "Error running generate multi references"
add_multiRefs || die "Error running add multi references"
generate || die "Failed to generate stuff"
stage "tables2" || die "Error running stage tables2"
stage 2 || die "Error running stage 2"
add_multiRefs_functions || die "Error running add multi references functions"
add_merge_link_triggers || die "Error running add merge link triggers"
