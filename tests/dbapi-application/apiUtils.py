import datetime
import time
import os

class CurrentTimestamp(object):
    def __init__(self):
        self.start = datetime.datetime.now() - datetime.timedelta(seconds=1)

    def __eq__(self, other):
        if not isinstance(other,str):
            raise TypeError, "Cannot compare CurrentTimestamp with anything but string"
        otherDate = datetime.datetime(*(time.strptime(other[:19], "%Y-%m-%d %H:%M:%S")[0:6]))
        # The start time, as reported by the remote server, uses one second
        # granularity, so better allow for one second earlier
        start = self.start - datetime.timedelta(seconds=1)
        end = datetime.datetime.now() + datetime.timedelta(seconds=1)
        return start <= otherDate and otherDate <= end

class DeskaDbUser(object):
    def __init__(self):
        self.user = os.environ["DESKA_USER"]

    def __eq__(self, other):
        if not isinstance(other,str):
            raise TypeError, "Cannot compare DeskaDbUser with anything but string"
        return other == self.user

    def __repr__(self):
        return "<%s: %s>" % (type(self).__name__, repr(self.user))

class AnyOrderList(object):
    def __init__(self, items):
        self.items = sorted(items)

    def __eq__(self, other):
        if not isinstance(other, list):
            raise TypeError, "Cannot compare AnyOrderList with anything but list"
        return self.items == sorted(other)

    def __repr__(self):
        return "<%s: %s>" % (type(self).__name__, repr(self.items))

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

class InvalidKindError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "InvalidKindError")

class InvalidAttributeError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "InvalidAttributeError")

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

class RevisionParsingError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "RevisionParsingError")

class RevisionRangeError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "RevisionRangeError")

class ChangesetParsingError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ChangesetParsingError")

class ChangesetRangeError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ChangesetRangeError")

class ConstraintError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ConstraintError")

class ObsoleteParentError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ObsoleteParentError")

class NotASetError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "NotASetError")

class ChangesetLockingError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ChangesetLockingError")

class CfgGeneratingError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "CfgGeneratingError")

class SpecialReadOnlyAttributeError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "SpecialReadOnlyAttributeError")

class SqlError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "SqlError")

class ServerError(RemoteDbException):
    def __init__(self):
        RemoteDbException.__init__(self, "ServerError")


def revisionIncrement(revision, change):
    return "r%d" % (int(revision[1:len(revision)]) + change)

class ApiMethod(object):
    counter = 0
    def __init__(self, name, args):
        tag = ".t%d" % ApiMethod.counter
        ApiMethod.counter += 1
        self.name = name
        self.command = {"command": name, "tag": tag}
        self.response = {"response": name, "tag": tag}
        if args is not None:
            self.command.update(args)

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

def pendingChangesets(filter=None):
    if filter is None:
        args = None
    else:
        args = {"filter": filter}
    return ApiMethod("pendingChangesets", args)

def resumeChangeset(changeset):
    return ApiMethod("resumeChangeset", {"changeset": changeset})

def detachFromCurrentChangeset(message):
    return ApiMethod("detachFromCurrentChangeset", {"message": message})

def abortCurrentChangeset():
    return ApiMethod("abortCurrentChangeset", None)

def createObject(kindName, objectName):
    return ApiMethod("createObject", {"kindName": kindName, "objectName":
                                      objectName})

def setAttribute(kindName, objectName, attributeName, attributeData):
    return ApiMethod("setAttribute", {"kindName": kindName, "objectName":
                                      objectName, "attributeName":
                                      attributeName,
                                      "attributeData": attributeData})

def setAttributeInsert(kindName, objectName, attributeName, attributeData):
    return ApiMethod("setAttributeInsert", {"kindName": kindName, "objectName":
                                      objectName, "attributeName":
                                      attributeName,
                                      "attributeData": attributeData})

def setAttributeRemove(kindName, objectName, attributeName, attributeData):
    return ApiMethod("setAttributeRemove", {"kindName": kindName, "objectName":
                                      objectName, "attributeName":
                                      attributeName,
                                      "attributeData": attributeData})

def renameObject(kindName, oldObjectName, newObjectName):
    return ApiMethod("renameObject", {"kindName": kindName,
                                      "oldObjectName": oldObjectName,
                                      "newObjectName": newObjectName})

def deleteObject(kindName, objectName):
    return ApiMethod("deleteObject", {"kindName": kindName, "objectName":
                                      objectName})

def restoreDeletedObject(kindName, objectName):
    return ApiMethod("restoreDeletedObject", {"kindName": kindName, "objectName":
                                      objectName})

def objectData(kindName, objectName, revision=None):
    args = {"kindName": kindName, "objectName": objectName}
    if revision is not None:
        args["revision"] = revision
    return ApiMethod("objectData", args)

def resolvedObjectData(kindName, objectName, revision=None):
    args = {"kindName": kindName, "objectName": objectName}
    if revision is not None:
        args["revision"] = revision
    return ApiMethod("resolvedObjectData", args)

def resolvedObjectDataWithOrigin(kindName, objectName, revision=None):
    args = {"kindName": kindName, "objectName": objectName}
    if revision is not None:
        args["revision"] = revision
    return ApiMethod("resolvedObjectDataWithOrigin", args)

def multipleObjectData(kindName, revision=None, filter=None):
    args = {"kindName": kindName}
    if revision is not None:
        args["revision"] = revision
    if filter is not None:
        args["filter"] = filter
    return ApiMethod("multipleObjectData", args)

def multipleResolvedObjectData(kindName, revision=None, filter=None):
    args = {"kindName": kindName}
    if revision is not None:
        args["revision"] = revision
    if filter is not None:
        args["filter"] = filter
    return ApiMethod("multipleResolvedObjectData", args)

def multipleResolvedObjectDataWithOrigin(kindName, revision=None, filter=None):
    args = {"kindName": kindName}
    if revision is not None:
        args["revision"] = revision
    if filter is not None:
        args["filter"] = filter
    return ApiMethod("multipleResolvedObjectDataWithOrigin", args)


def verifyingObjectMultipleData(r, kindName, objectName):
    one = r.c(objectData(kindName, objectName))
    multiple = r.c(multipleObjectData(kindName,
                    filter={"condition": "columnEq", "kind": kindName,
                            "attribute":"name", "value": objectName}))
    r.assertTrue(len(multiple), 1)
    r.assertTrue(multiple.has_key(objectName))
    r.assertEqual(one, multiple[objectName])
    r.assertEqual(r.c(kindInstances(kindName,
        filter={"condition": "columnEq", "kind": kindName,
                "attribute":"name", "value": objectName})),
                  [objectName])
    return one

def kindInstances(kindName, revision=None, filter=None):
    args = {"kindName": kindName}
    if revision is not None:
        args["revision"] = revision
    if filter is not None:
        args["filter"] = filter
    return ApiMethod("kindInstances", args)

def listRevisions(filter=None):
    args = {}
    if filter is not None:
        args["filter"] = filter
    return ApiMethod("listRevisions", args)

def showConfigDiff(forceRegen=False):
    return ApiMethod("showConfigDiff", {"forceRegen": forceRegen})

def dataDifference(revisionA, revisionB):
    return ApiMethod("dataDifference", {"revisionA": revisionA, "revisionB": revisionB})

def dataDifferenceInTemporaryChangeset(changeset):
    return ApiMethod("dataDifferenceInTemporaryChangeset", {"changeset": changeset})

def resolvedDataDifference(revisionA, revisionB):
    return ApiMethod("resolvedDataDifference", {"revisionA": revisionA, "revisionB": revisionB})

def resolvedDataDifferenceInTemporaryChangeset(changeset):
    return ApiMethod("resolvedDataDifferenceInTemporaryChangeset", {"changeset": changeset})

def freezeView():
    return ApiMethod("freezeView", None)

def unFreezeView():
    return ApiMethod("unFreezeView", None)

def lockCurrentChangeset():
    return ApiMethod("lockCurrentChangeset", None)

def unlockCurrentChangeset():
    return ApiMethod("unlockCurrentChangeset", None)
