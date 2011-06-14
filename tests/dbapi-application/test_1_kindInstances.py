from apiUtils import *

def imperative(r):
    objectNames = set(["test1", "test2", "test3"])
    firstKI = r.c(kindInstances("vendor"))
    objectNames = objectNames - set(firstKI)

    r.c(startChangeset())
    for obj in objectNames:
        r.c(createObject("vendor", obj))

    changesetKI = r.c(kindInstances("vendor"))
    # changesetKI should containt whole objectNames
    r.assertTrue(set(objectNames) <= set(changesetKI))

    revision = r.c(commitChangeset("test"))
    newKI = r.c(kindInstances("vendor", revision))
    r.assertTrue(objectNames <= set(newKI))

    revision = revisionIncrement(revision, -1)
    revNewKI = r.c(kindInstances("vendor", revision))
    r.assertEquals(revNewKI, firstKI)
