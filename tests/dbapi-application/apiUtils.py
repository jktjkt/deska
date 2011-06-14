import datetime
import os

class CurrentTimestamp(object):
    def __init__(self):
        self.start = datetime.datetime.now() - datetime.timedelta(seconds=1)

    def __eq__(self, other):
        if not isinstance(other,str):
            raise TypeError, "Cannot compare CurrentTimestamp with anything but string"
        otherDate = datetime.datetime.strptime(other[:19], "%Y-%m-%d %H:%M:%S")
        end = datetime.datetime.now() + datetime.timedelta(seconds=1)
        return self.start <= otherDate and otherDate <= end

class DeskaDbUser(object):
    def __init__(self):
        self.user = os.environ["DESKA_USER"]

    def __eq__(self, other):
        if not isinstance(other,str):
            raise TypeError, "Cannot compare DeskaDbUser with anything but string"
        return other == self.user

class AnyOrderList(object):
    def __init__(self, items):
        self.items = frozenset(items)

    def __eq__(self, other):
        if not isinstance(other, list):
            raise TypeError, "Cannot compare AnyOrderList with anything but list"
        return self.items == frozenset(other)

class Any(object):
    """Compare against anything with True result"""
    def __eq__(self, other):
        return True

class ListEnd(object):
    """Make sure that the data are present at the end of the other object"""
    def __init__(self, data):
        self.data = data

    def __eq__(self, other):
        subset = other[-len(self.data):]
        return subset == self.data

    def __repr__(self):
        return "<%s: %s>" % (type(self).__name__, repr(self.data))

class RemoteDbException(object):
    def __init__(self, name):
        self.name = name

    def __eq__(self, other):
        if not isinstance(other, dict):
            raise TypeError, "Cannot compare DB exception %s" % self.name
        if sorted(other.keys()) != ["message", "type"]:
            raise ValueError, "Exception has weird keys: %s" % repr(other)
        if not isinstance(other["message"], str):
            raise TypeError, "Weird exception message"
        if not len(other["message"]):
            raise ValueError, "Message too short: %s" % repr(other["message"])
        return other["type"] == self.name

class NotFoundError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "NotFoundError")

class NoChangesetError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "NoChangesetError")

class ChangesetAlreadyOpenError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ChangesetAlreadyOpenError")

class FreezingError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "FreezingError")

class FilterError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "FilterError")

class ReCreateObjectError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ReCreateObjectError")

class InvalidKindError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "InvalidKindError")

class InvalidAttributeError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "InvalidAttributeError")

class RevisionParsingError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "RevisionParsingError")

class ChangesetParsingError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ChangesetParsingError")

class SqlError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "SqlError")

class ServerError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ServerError")


def revisionIncrement(revision, change):
    return "r{0}".format(int(revision[1:len(revision)]) + change)

registeredVariables = {}
'''Storage of assigned objects for later checks'''


class FillPlaceholder(object):
    '''Fill in a placeholder variable when performing a comparison'''

    def __init__(self, name):
        registeredVariables[name] = None
        self.name = name

    def __eq__(self, other):
        registeredVariables[self.name] = other
        return True


class Variable(object):
    '''Check against previously obtained value'''

    def __init__(self, name):
        self.name = name

    def __eq__(self, other):
        return registeredVariables[self.name] == other

    def __repr__(self):
        if self.name in registeredVariables:
            return "<%s: %s>" % (
                type(self).__name__, registeredVariables[self.name])
        else:
            return "<%s: %s>" % (type(self).__name__, "<not assigned>")

    def get(self):
        return registeredVariables[self.name]


class ApiMethod(object):
    def __init__(self, name, args):
        self.name = name
        self.command = {"command": name}
        self.response = {"response": name}
        if args is not None:
            self.command.update(args)
            self.response.update(args)

    def returns(self, value):
        self.response[self.name] = value
        return self

    def throws(self, exception):
        self.response["dbException"] = exception
        return self

    def register(self, name):
        self.response[self.name] = FillPlaceholder(name)
        return self

    def __eq__(self, other):
        return (self.command, self.response) == other

    def __repr__(self):
        return "<%s: %s, %s>" % (
            type(self).__name__, str(self.command), str(self.response))


# DBAPI commands
def kindNames():
    return ApiMethod("kindNames", None)

def kindAttributes(kindName):
    return ApiMethod("kindAttributes", {"kindName": kindName})

def kindRelations(kindName):
    return ApiMethod("kindRelations", {"kindName": kindName})

def startChangeset():
    return ApiMethod("startChangeset", None)

def commitChangeset(message):
    return ApiMethod("commitChangeset", {"commitMessage": message})

def rebaseChangeset(parentRevision):
    return ApiMethod("rebaseChangeset", {"parentRevision": parentRevision})

def pendingChangesets():
    # FIXME: filter
    return ApiMethod("pendingChangesets", None)

def resumeChangeset(revision):
    return ApiMethod("resumeChangeset", {"changeset": revision})

def detachFromCurrentChangeset(message):
    return ApiMethod("detachFromCurrentChangeset", {"message": message})

def abortCurrentChangeset():
    return ApiMethod("abortCurrentChangeset", None)

def createObject(kindName, objectName):
    return ApiMethod("createObject", {"kindName": kindName, "objectName":
                                      objectName})

def kindInstances(kindName, revision=None):
    # FIXME: filter
    args = {"kindName": kindName}
    if revision is not None:
        args["revision"] = revision
    return ApiMethod("kindInstances", args)

def listRevisions(filter=None):
    args = {}
    if filter is not None:
        args["filter"] = filter
    return ApiMethod("listRevisions", args)
