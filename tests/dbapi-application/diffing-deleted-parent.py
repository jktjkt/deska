from apiUtils import *

def imperative(r):
    # Redmine #316, checking that the objects in a changeset are always reported with fully
    # quallified names
    tmp1 = r.c(startChangeset())
    r.cvoid(detachFromCurrentChangeset("empty"))
    tmp2 = r.c(startChangeset())
    r.c(createObject("host", "b"))
    r.c(commitChangeset("created b"))

    r.cvoid(resumeChangeset(tmp1))
    r.c(createObject("host", "a"))
    r.c(createObject("interface", "a->i0"))
    r.cvoid(deleteObject("host", "a"))
    expectedDiff = [
        {'command': 'createObject', 'kindName': 'interface', 'objectName': 'a->i0'}
    ]
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp1)), expectedDiff)

    r.cvoid(detachFromCurrentChangeset("will rebase"))
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp1)), expectedDiff)
    tmp3 = r.c(startChangeset())
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp1)), expectedDiff)
    r.cvoid(detachFromCurrentChangeset("temporarily going away"))
    r.cvoid(resumeChangeset(tmp1))
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp1)), expectedDiff)
