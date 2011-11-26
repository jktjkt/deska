#!/bin/bash

die() {
    echo "${1}"
    exit 33
}

if [[ -z "${1}" ]]; then
    echo "Usage: ${0} path/to/put/git/stuff/into"
    exit 33
fi

if [[ ! -d "${1}" ]]; then
    echo "FAIL: parameter has to be an already-existing empty directory"
    exit 33
fi

if [[ -n $(ls "${1}") ]]; then
    echo "FAIL: directory ${1} is not empty"
    exit 33
fi

source ./tests/sql/util-manage-git.sh
deska_init_git "${1}" || die "FAIL: cannot prepare git generators"

SAMPLESCRIPT="${DESKA_CFGGEN_SCRIPTS}/01-demo"
cat > "${SAMPLESCRIPT}" << EOF
#!/usr/bin/python
import deska

deska.init()

output = file("all-hosts", "wb")
for hostname, data in deska.host._all().iteritems:
    output.write("%s: %s\\n" % (hostname, data["service"]))
EOF

chmod +x "${SAMPLESCRIPT}"


echo
echo
echo "Please copy-paste the following to your shell now:"
echo
echo "export DESKA_CFGGEN_BACKEND=${DESKA_CFGGEN_BACKEND};" \
    "export DESKA_CFGGEN_SCRIPTS=${DESKA_CFGGEN_SCRIPTS};" \
    "export DESKA_CFGGEN_GIT_REPO=${DESKA_CFGGEN_GIT_REPO};" \
    "export DESKA_CFGGEN_GIT_PRIMARY_CLONE=${DESKA_CFGGEN_GIT_PRIMARY_CLONE};"\
    "export DESKA_CFGGEN_GIT_SECOND=${DESKA_CFGGEN_GIT_SECOND};"\
    "export DESKA_CFGGEN_GIT_WC=${DESKA_CFGGEN_GIT_WC};"
echo
echo
echo "A sample script has been put into ${SAMPLESCRIPT} for you."
