#/bin/bash

die() {
    echo $1 failed
    exit 6
}

SU=postgres
DB=nightly_deska
USER=foobar
