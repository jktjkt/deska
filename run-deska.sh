#!/bin/bash

#export DESKA_SERVER_IO_TRACE=1

export DESKA_DB=d_fzu
#export DESKA_DB=d_demo
export DESKA_CFGGEN_BACKEND=git
export DESKA_CFGGEN_SCRIPTS=/home/jkt/work/fzu/deska/_build/cfggen-testing/scripts
export DESKA_CFGGEN_GIT_REPO=/home/jkt/work/fzu/deska/_build/cfggen-testing/cfggen-repo
export DESKA_CFGGEN_GIT_WC=/home/jkt/work/fzu/deska/_build/cfggen-testing/cfggen-wc
export DESKA_CFGGEN_GIT_PRIMARY_CLONE=/home/jkt/work/fzu/deska/_build/cfggen-testing/cfggen-primary
export DESKA_CFGGEN_GIT_SECOND=/home/jkt/work/fzu/deska/_build/cfggen-testing/second-wd
~/work/fzu/deska/src/deska/server/app/deska-server
