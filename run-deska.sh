#!/bin/bash

# This is a sample script for running the Deska server over SSH. Simply adjust
# these variables and point the Deska CLI by its deska.ini file to this file.
# Exmaple:
#
# [DBConnection]
# Server=ssh
# Server=localhost
# Server=~/work/fzu/deska/run-deska.sh
#
# (Yes, that's three lines assigning to the "Server" variable. That's how Boost
# works.
#
# You will very likely have to adjust these paths in your system.

# Enable this one if you want to produce a machine-readable log of the IO communication
#export DESKA_SERVER_IO_TRACE=1

# The database scheme to use
export DESKA_DB=d_fzu
export DESKA_DB=d_demo

# Enable configuration generators. The tests/prepare-git-repo.sh script can be
# used to set up this directory tree, and sample scripts in the
# scripts/config-generators directory can be symlinked into the
# $DESKA_CFGGEN_SCRIPTS directory to activate those currently in use at the FZU.
export DESKA_CFGGEN_BACKEND=git
export DESKA_CFGGEN_BACKEND=fake
export DESKA_CFGGEN_SCRIPTS=/home/jkt/work/fzu/deska/_build/cfggen-testing/scripts
export DESKA_CFGGEN_GIT_REPO=/home/jkt/work/fzu/deska/_build/cfggen-testing/cfggen-repo
export DESKA_CFGGEN_GIT_WC=/home/jkt/work/fzu/deska/_build/cfggen-testing/cfggen-wc
export DESKA_CFGGEN_GIT_PRIMARY_CLONE=/home/jkt/work/fzu/deska/_build/cfggen-testing/cfggen-primary
export DESKA_CFGGEN_GIT_SECOND=/home/jkt/work/fzu/deska/_build/cfggen-testing/second-wd

# Now that we have set up our environment, launch the server.
~/work/fzu/deska/src/deska/server/app/deska-server
