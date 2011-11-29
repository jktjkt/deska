#!/bin/sh

cd "${1}"
echo "const char* deskaGitVersion=\"`git describe --dirty --long || git describe --long`\";" > "${2}"
