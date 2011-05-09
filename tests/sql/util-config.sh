#!/bin/bash

die() {
    echo $1 failed
    exit 6
}

DESKA_SU=${DESKA_SU:-postgres}
DESKA_DB=${DESKA_DB:-nightly_deska}
DESKA_USER=${DESKA_USER:-foobar}

. ./util-cmake-options.sh || die "Reading configuration from CMake"
