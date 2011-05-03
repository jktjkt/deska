#!/bin/sh

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
	psql -d "$DATABASE" -U "$USER" -f drop_$1.sql 2>&1 > /dev/null | grep -v "cascades"
}

function stage(){
	echo "Stage $1 ..."
	psql -d "$DATABASE" -U "$USER" -f create_$1.sql -v dbname="$DATABASE" 2>&1 > /dev/null | grep -v NOTICE | grep -v "current transaction is aborted"
}

function generate(){
	echo "Generating stored procedures ..."
	python gen_sql/generator.py
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
	echo "Drop DB"
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
	echo "Install DB"
	if test $TYPE == "A"
	then
		stage "tables"
		stage 0
	fi
	stage 1
	generate
	stage 2
fi

if test $ACTION == "F"
then
	echo "Regenerate DB functions"
	drop 2
	drop 0
	stage 0
	stage 2
fi

if test $ACTION == "R"
then
	echo "Restore DB"
	drop 2
	drop 1
	if test $TYPE == "A"
	then
		drop 0
		drop "tables"
		stage "tables"
		stage 0
	fi
	stage 1
	generate
	stage 2
fi
