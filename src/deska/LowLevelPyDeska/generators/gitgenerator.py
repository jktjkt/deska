'''A git-based implementation of the configuration generators helper'''

# FIXME: race conditions when multiple sessions work on the same changeset...

import os
import stat
import git

GIT_NEW_WORKDIR="/usr/share/git/contrib/workdir/git-new-workdir"

class GitGenerator(object):
    '''Encapsulate access to the git repository'''

    def __init__(self, repodir, workdir, scriptdir):
        self.repodir = repodir
        self.workdir = workdir
        self.scriptdir = scriptdir
        self.git = git.Git(self.workdir)

    def openRepo(self):
        '''Make sure that the working copy is usable and initialized to a clean state'''
        if not os.path.exists(self.workdir):
            self.newCheckout()
        else:
            self.clean()

    def newCheckout(self):
        '''Call git-new-workdir to set up a new working copy'''
        git.Git().execute([GIT_NEW_WORKDIR, self.repodir, self.workdir])

    def clean(self):
        '''Clean a pre-existing working directory'''
        self.git.reset("--hard", "HEAD")
        self.git.clean("-d", "-f")

    def diff(self):
        '''Return a human-readable diff of the changes'''
        self.git.add("-A")
        return self.git.diff("--color", "--staged")

    def save(self, message):
        '''Commit and push the changes into a persistent location'''
        self.git.add("-A")
        self.git.commit("-m", message)
        self.git.push()

    def generate(self):
        '''Actually run the config generators'''

        runAnything = False

        # enumerate all files in the target directory
        for script in sorted(os.listdir(self.scriptdir)):
            path = os.path.join(self.scriptdir, script)
            s = os.stat(path)
            if stat.S_ISDIR(s.st_mode):
                # FIXME: error handling
                continue
            if not s.st_mode & stat.S_IXUSR:
                # Ignore scripts without an exectuable bit. This is on purpose,
                # so that we can safely ignore a README etc.
                continue
            # now just abuse all the infrastructure around the git wrapper to
            # execute the command
            git.Git(self.workdir).execute([path])
            runAnything = True

        if not runAnything:
            raise RuntimeError, "No executable scripts found in %s" % self.scriptdir

    def apiConfigDiff(self, changesetIsFresh):
        '''Return a human-readable diff of the output, no matter of the initial state'''
        if not changesetIsFresh:
            self.openRepo()
            self.generate()
        return self.diff()

    def apiSave(self, changesetIsFresh, message):
        '''Push the new output to the repository, regenerating stuff if needed'''
        if not changesetIsFresh:
            self.openRepo()
            self.generate()
        self.save(message)
