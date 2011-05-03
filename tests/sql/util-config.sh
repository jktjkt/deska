#/bin/bash

die() {
    echo $1 failed
    exit 6
}

DESKA_SU=postgres
DESKA_DB=nightly_deska
DESKA_USER=foobar
