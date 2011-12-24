'''A git-based implementation of the configuration generators helper'''

import os
import stat
import shutil
import git
from generatorerror import GeneratorError

GIT_NEW_WORKDIR=os.path.join(os.path.dirname(__file__), "git-new-workdir")

class CommandExecutor(object):
    def executeScript(self, command, workdir):
        # now just abuse all the infrastructure around the git wrapper to
        # execute the command
        git.Git(workdir).execute([command])

class GitGenerator(object):
    '''Encapsulate access to the git repository'''

    def __init__(self, repodir, workdir, scriptdir):
        self.repodir = repodir
        self.workdir = workdir
        self.scriptdir = scriptdir
        self.git = git.Git(self.workdir)
        # Pull from the upsteram repository before we do anything
        # Redmine #419
        repogit = git.Git(self.repodir)
        # A push might have failed already, so we better switch to the upstream
        # master at first.  We shall not lose any data here, as the config
        # generators are stateless anyway, and if this repository has diverged
        # from upstream, it means that the `git pull` has surely failed, which
        # means that the changes could not possibly be commited to the DB
        # either.
        repogit.reset("--hard", "origin/master")
        # Now refresh our copy
        repogit.pull()

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
        # Git 1.6.6 does not support the `git status --porcelain` output (Redmine #347).
        # Let's hope this is enough.
        try:
            gitstatus = self.git.status("--porcelain")
        except git.errors.GitCommandError, e:
            if e.status == 129:
                gitstatus = self.git.diff("--cached", "--name-only")
            else:
                raise
        if gitstatus == "":
            # no changes to commit
            pass
        else:
            self.git.commit("-m", message)
        self.git.push()

    def generate(self, executor):
        '''Actually run the config generators'''

        runAnything = False

        # enumerate all files in the target directory
        for script in sorted(os.listdir(self.scriptdir)):
            path = os.path.join(self.scriptdir, script)
            s = os.stat(path)
            if stat.S_ISDIR(s.st_mode):
                # Directories are ignored -- maybe they are required by the
                # scripts after all...
                continue
            if not s.st_mode & stat.S_IXUSR:
                # Ignore scripts without an exectuable bit. This is on purpose,
                # so that we can safely ignore a README etc.
                continue
            executor.executeScript(path, self.workdir)
            runAnything = True

        if not runAnything:
            raise GeneratorError, "No executable scripts found in %s" % self.scriptdir

    def nukeWorkDir(self):
        shutil.rmtree(self.workdir)

    def completeConfigDiff(self, changesetIsFresh):
        '''Return a human-readable diff of the output, no matter of the initial state'''
        if not changesetIsFresh:
            self.openRepo()
            self.generate(CommandExecutor())
        return self.diff()

    def completeSave(self, changesetIsFresh, message):
        '''Push the new output to the repository, regenerating stuff if needed'''
        if not changesetIsFresh:
            self.openRepo()
            self.generate(CommandExecutor())
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
