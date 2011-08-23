'''Test createObject() and server-side assignment of identifier names'''

from apiUtils import *

def imperative(r):
    # prepare the environment
    r.c(startChangeset())
    # create one host for embedding
    r.assertEqual(r.c(createObject("host", "foo")), "foo")

    # The very first action: let the DB assign a persistent name
    r.assertEqual(r.c(createObject("interface", "foo->")), "foo->interface_1")
    # Trying it once again should return a new one
    r.assertEqual(r.c(createObject("interface", "foo->")), "foo->interface_2")
    # try to put a hole in between
    r.assertEqual(r.c(createObject("interface", "foo->bar")), "foo->bar")
    # the number should not change by that
    r.assertEqual(r.c(createObject("interface", "foo->")), "foo->interface_3")
    # on the other hand, creating one with the name same as the automatic one should work
    r.assertEqual(r.c(createObject("interface", "foo->interface_4")), "foo->interface_4")
    # ...but it will, of course, affect the numeric sequence
    r.assertEqual(r.c(createObject("interface", "foo->")), "foo->interface_5")

    # now let's detach, start a new changeset and see how the sequences behave
    r.cvoid(detachFromCurrentChangeset("blah"))
    r.c(startChangeset())
    r.assertEqual(r.c(createObject("host", "foo")), "foo")

    # it should be one again
    r.assertEqual(r.c(createObject("interface", "foo->")), "foo->interface_1")

    # save the result, start a new changeset and test further
    r.c(commitChangeset("foo"))
    r.c(startChangeset())

    # we've committed number one already
    r.assertEqual(r.c(createObject("interface", "foo->")), "foo->interface_2")

    # creating non-embedded objects with an empty name should fail
    # FAILS, Redmine#274
    #r.cfail(createObject("host", ""))
