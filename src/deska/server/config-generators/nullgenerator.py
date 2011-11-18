'''A fake implementation of the configuration generators helper that does absolutely nothing'''

class NullGenerator(object):
    '''Encapsulate access to the git repository'''

    def __init__(self, behavior):
        self.behavior = behavior

    def maybeThrow(self):
        if self.behavior is not None:
            raise self.behavior

    def openRepo(self):
        self.maybeThrow()

    def diff(self):
        self.maybeThrow()
        return "NullGenerator: configuration generators were not configured yet"

    def apiSave(self, message):
        self.maybeThrow()

    def generate(self, executor):
        self.maybeThrow()
