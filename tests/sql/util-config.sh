#!/bin/bash

die() {
    echo $1 failed
    exit 6
}

DESKA_SU=${DESKA_SU:-postgres}
DESKA_DB=${DESKA_DB:-nightly_deska}
DESKA_ADMIN=${DESKA_USER:-pwnzor}
DESKA_USER=${DESKA_USER:-foobar}
DESKA_TABLESPACE=${DESKA_TABLESPACE:-}

. ./util-cmake-options.sh || die "Reading configuration from CMake"
