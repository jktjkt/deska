'''A fake implementation of the configuration generators helper that does absolutely nothing'''

class NullGenerator(object):
    '''Encapsulate access to the git repository'''

    def openRepo(self):
        pass

    def diff(self):
        return "NullGenerator: configuration generators were not configured yet"

    def apiSave(self, message):
        pass

    def generate(self):
        pass
