deska_git_die() {
    echo "${1}"
    exit 3
}

deska_init_git()
{
    MY_PREFIX="${1}"
    if [[ -z "${MY_PREFIX}" ]]; then
        deska_git_die "Usage: deska_init_git path/to/the/directory"
    fi

    DESKA_CFGGEN_BACKEND=git
    export DESKA_CFGGEN_BACKEND
    DESKA_CFGGEN_GIT_REPO=${MY_PREFIX}/cfggen-repo
    export DESKA_CFGGEN_GIT_REPO
    DESKA_CFGGEN_GIT_PRIMARY_CLONE=${MY_PREFIX}/cfggen-primary
    export DESKA_CFGGEN_GIT_PRIMARY_CLONE
    DESKA_CFGGEN_GIT_WC=${MY_PREFIX}/cfggen-wc
    export DESKA_CFGGEN_GIT_WC
    DESKA_CFGGEN_SCRIPTS=${MY_PREFIX}/scripts
    export DESKA_CFGGEN_SCRIPTS
    DESKA_CFGGEN_GIT_SECOND=${MY_PREFIX}/second-wd
    export DESKA_CFGGEN_GIT_SECOND

    export GIT_AUTHOR_NAME="Unit Test"
    export GIT_AUTHOR_EMAIL="unit.test@example.org"

    # Initialize the master repository which simulates a remote repo
    git init --bare --shared=0666 ${DESKA_CFGGEN_GIT_REPO} || deska_git_die "git init --bare my_repo failed"

    # Set up a repo-side hook for auto-updating the second clone
    echo -e "#!/bin/bash\numask 000\nGIT_WORK_TREE="${DESKA_CFGGEN_GIT_SECOND}" git checkout -f" > \
        "${DESKA_CFGGEN_GIT_REPO}/hooks/post-receive"
    chmod +x "${DESKA_CFGGEN_GIT_REPO}/hooks/post-receive"
    mkdir "${DESKA_CFGGEN_GIT_SECOND}" || deska_git_die "creating the DESKA_CFGGEN_GIT_SECOND failed"

    umask 000
    # Initialize the "primary clone"; that's the repository from which we create extra WCs
    git clone ${DESKA_CFGGEN_GIT_REPO} ${DESKA_CFGGEN_GIT_PRIMARY_CLONE} ||
        deska_git_die "git clone my_repo my_primary_clone failed"
    # Got to start the master branch now, or git-new-workdir will break
    pushd ${DESKA_CFGGEN_GIT_PRIMARY_CLONE}
    echo "This is a repo of the resulting configuration" > README
    git add README || deska_git_die "git add README failed"
    git commit -m "Initial commit" || deska_git_die "git commit failed"
    git push origin master || deska_git_die "git push origin master failed"
    popd
    umask 022
    chmod 777 "${DESKA_CFGGEN_GIT_PRIMARY_CLONE}"

    # Prepare (empty) generating scripts
    mkdir "${DESKA_CFGGEN_SCRIPTS}"

    # Prepare the root for the working directories (one per changeset)
    mkdir "${DESKA_CFGGEN_GIT_WC}"
    chmod 0777 "${DESKA_CFGGEN_GIT_WC}"
}
