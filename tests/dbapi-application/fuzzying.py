'''Tests for "invalid" argument values'''

from apiUtils import *

def imperative(r):
    nastyNames = (" space", " ", "' blah", ";", ",", "", " -- ")

    for x in nastyNames:
        r.cfail(kindAttributes(x), InvalidKindError())
        r.cfail(kindRelations(x), InvalidKindError())
        r.cfail(kindInstances(x), InvalidKindError())

        r.cfail(objectData(x, "foo"), InvalidKindError())
        r.cfail(resolvedObjectData(x, "foo"), InvalidKindError())
        r.cfail(resolvedObjectDataWithOrigin(x, "foo"), InvalidKindError())
        r.cfail(multipleObjectData(x), InvalidKindError())
        r.cfail(multipleResolvedObjectData(x), InvalidKindError())
        r.cfail(multipleResolvedObjectDataWithOrigin(x), InvalidKindError())

        r.cfail(objectData("hardware", x), NotFoundError())
        r.cfail(resolvedObjectData("hardware", x), NotFoundError())

        filters = [
            {"kind": x, "condition": "columnEq", "attribute": "x", "value": None},
            {"kind": "hardware", "condition": x, "attribute": "x", "value": None},
            {"kind": "hardware", "condition": "columnEq", "attribute": x, "value": None},
            {"kind": "hardware", "condition": "columnEq", "attribute": "x", "value": None},
            {"metadata": x, "condition": "columnEq", "value": None},
            {"metadata": "author", "condition": x, "value": None},
        ]
        for y in filters:
            r.cfail(multipleObjectData("hardware", filter=y), FilterError())
            r.cfail(multipleResolvedObjectData("hardware", filter=y), FilterError())
            r.cfail(multipleResolvedObjectDataWithOrigin("hardware", filter=y), FilterError())

    r.c(startChangeset())
    r.c(createObject("hardware", "dummy"))
    r.c(createObject("host", "dummy"))
    r.c(createObject("service", "dummy"))
    for x in nastyNames:
        # kinds
        r.cfail(createObject(x, "a"), InvalidKindError())
        r.cfail(deleteObject(x, "a"), InvalidKindError())
        r.cfail(restoreDeletedObject(x, "a"), InvalidKindError())
        r.cfail(renameObject(x, "a", "b"), InvalidKindError())
        r.cfail(setAttribute(x, "a", "b", None), InvalidKindError())
        r.cfail(setAttributeInsert(x, "a", "service", "dummy"), InvalidKindError())
        r.cfail(setAttributeRemove(x, "a", "service", "dummy"), InvalidKindError())

        # object names
        r.cfail(createObject("hardware", x), ConstraintError())
        r.cfail(deleteObject("hardware", x), NotFoundError())
        r.cfail(restoreDeletedObject("hardware", x), NotFoundError())
        r.cfail(renameObject("hardware", x, "b"), NotFoundError())
        r.cfail(renameObject("hardware", "dummy", x), ConstraintError())
        r.cfail(setAttribute("hardware", x, "note_hardware", None), NotFoundError())
        r.cfail(setAttributeInsert("host", x, "service", "dummy"), NotFoundError())
        r.cfail(setAttributeRemove("host", x, "service", "dummy"), NotFoundError())

        # attribute names
        r.cfail(setAttribute("hardware", "dummy", x, None), InvalidAttributeError())
        r.cfail(setAttributeInsert("host", "dummy", x, "dummy"), InvalidAttributeError())
        r.cfail(setAttributeRemove("host", "dummy", x, "dummy"), InvalidAttributeError())

        # attribute values for sets
        r.cfail(setAttributeInsert("host", "dummy", "service", x), NotFoundError())
        r.cfail(setAttributeRemove("host", "dummy", "service", x), NotFoundError())
        r.cfail(setAttribute("host", "dummy", "service", [x]), NotFoundError())

        # attribute values -- this should work
        r.cvoid(setAttribute("hardware", "dummy", "note_hardware", x))

    r.cvoid(abortCurrentChangeset())
    for x in nastyNames:
        r.cfail(resumeChangeset(x), ChangesetParsingError())
        changeset = r.c(startChangeset())
        if x == "":
            # FIXME: better exception?
            r.cfail(detachFromCurrentChangeset(x), ConstraintError())
            r.cvoid(detachFromCurrentChangeset("blah"))
            r.cvoid(resumeChangeset(changeset))
            r.cfail(commitChangeset(x), ConstraintError())
            r.c(commitChangeset("blah"))
        else:
            r.cvoid(detachFromCurrentChangeset(x))
            r.cvoid(resumeChangeset(changeset))
            r.c(commitChangeset(x))

    # FIXME: restoringCommit
    # FIXME: listRevisions and pendingChangesets
    # FIXME: dataDIfference (all variants)

