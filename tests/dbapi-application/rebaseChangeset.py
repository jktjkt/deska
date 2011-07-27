'''Test rebase changeset with pre-existing data'''

from apiUtils import *

def imperative(r):
    vendorNames = set(["vendor1", "vendor2"])
    presentVendors = set(r.c(kindInstances("vendor")))
    vendorNames = vendorNames - presentVendors
    
    changeset = r.c(startChangeset())
    for name in vendorNames:
        r.cvoid(createObject("vendor", name))
    r.cvoid(detachFromCurrentChangeset("test"))

    vendorNames = set(["vendor3", "vendor4"])
    presentVendors = set(r.c(kindInstances("vendor")))
    vendorNames = vendorNames - presentVendors

    r.c(startChangeset())
    for name in vendorNames:
        r.cvoid(createObject("vendor", name))
    revision = r.c(commitChangeset("test"))
    
    r.cvoid(resumeChangeset(changeset))
    r.cfail(commitChangeset("test"),ObsoleteParentError())
    