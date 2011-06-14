'''Test kindInstances behavior with pre-existing data'''

from apiUtils import *

def imperative(r):
    objectNames = set(["test1", "test2", "test3"])
    firstKI = set(r.c(kindInstances("vendor")))
    objectNames = objectNames - firstKI

    r.c(startChangeset())
    for obj in objectNames:
        r.c(createObject("vendor", obj))

    changesetKI = set(r.c(kindInstances("vendor")))
    # changesetKI should containt whole objectNames
    r.assertTrue(objectNames <= changesetKI)

    revision = r.c(commitChangeset("test"))
    newKI = set(r.c(kindInstances("vendor", revision)))
    r.assertTrue(objectNames <= newKI)

    revision = revisionIncrement(revision, -1)
    revNewKI = set(r.c(kindInstances("vendor", revision)))
    r.assertEquals(revNewKI, firstKI)
