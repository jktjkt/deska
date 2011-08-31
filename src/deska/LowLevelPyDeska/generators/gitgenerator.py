'''A git-based implementation of the configuration generators helper'''

# FIXME: race conditions when multiple sessions work on the same changeset...

import os
import stat
import git

GIT_NEW_WORKDIR=os.path.join(os.path.dirname(__file__), "git-new-workdir")

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

    def apiSave(self, message):
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

    def completeConfigDiff(self, changesetIsFresh):
        '''Return a human-readable diff of the output, no matter of the initial state'''
        if not changesetIsFresh:
            self.openRepo()
            self.generate()
        return self.diff()

    def completeSave(self, changesetIsFresh, message):
        '''Push the new output to the repository, regenerating stuff if needed'''
        if not changesetIsFresh:
            self.openRepo()
            self.generate()
        self.apiSave(message)

if __name__ == "__main__":
    import sys
    g = GitGenerator(sys.argv[1], sys.argv[2], sys.argv[3])
    offset = 4
    while offset < len(sys.argv):
        if sys.argv[offset] == "diff-fresh":
            print g.completeConfigDiff(True)
        elif sys.argv[offset] == "diff-nonfresh":
            print g.completeConfigDiff(False)
        elif sys.argv[offset] == "save-fresh":
            print g.completeSave(True, str(offset))
        elif sys.argv[offset] == "save-nonfresh":
            print g.completeSave(False, str(offset))
        else:
            raise ValueError, "Unsupported operator %s" % sys.argv[offset]
        offset += 1
