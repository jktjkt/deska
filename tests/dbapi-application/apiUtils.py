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

class ChangesetAlreadyOpenError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ChangesetAlreadyOpenError")

class NoChangesetError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "NoChangesetError")

class ServerError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ServerError")


class ApiMethod(object):
    def __init__(self, name, args):
        self.command = {"command": name}
        self.response = {"response": name}
        if args is not None:
            self.command.update(args)
            self.response.update(args)

    def returns(self, value):
        self.response[self.response["response"]] = value
        return self

    def throws(self, exception):
        self.response["dbException"] = exception
        return self

    def __eq__(self, other):
        return (self.command, self.response) == other

    def __repr__(self):
        return str(self.command, self.response)


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
