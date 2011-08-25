from apiUtils import *

def imperative(r):
    # at first, we can create a changeset
    r.c(startChangeset())
    # but trying that once again will fail
    r.cfail(startChangeset(), ChangesetAlreadyOpenError())
    # so let's abort the real changeset
    r.cvoid(abortCurrentChangeset())
    # we won't be able to abort it twice, though
    r.cfail(abortCurrentChangeset(), NoChangesetError())
    # so now we're outside of a changeset

    # try to commit, outside of a changeset
    r.cfail(commitChangeset("commit-outside-a-changeset"), NoChangesetError())

    # try to commit a resumed changeset
    changeset = r.c(startChangeset())
    r.cvoid(detachFromCurrentChangeset("detaching"))
    r.cfail(detachFromCurrentChangeset("detaching once again"), NoChangesetError())
    # FIXME: resumeChangeset("xyz"), ChangesetParsingError()),
    r.cfail(resumeChangeset("xyz"), ServerError())
    r.assertEqual(r.c(pendingChangesets()),
        ListEnd([
            {"status": "DETACHED", "changeset": changeset,
             "author": DeskaDbUser(), "timestamp": CurrentTimestamp(),
             "parentRevision": Any(), "message": "detaching"}]))
    # FIXME: call pendingChangesets with a filter
    r.cvoid(resumeChangeset(changeset))
    r.cfail(resumeChangeset(changeset), ChangesetAlreadyOpenError())
    v1 = r.c(commitChangeset("commit-resumed"))

    # try to commit a fresh one
    r.c(startChangeset())
    v2 = r.c(commitChangeset("commit-fresh"))
    vlist = [
        {"timestamp": CurrentTimestamp(), "commitMessage": "commit-resumed",
         "revision": v1, "author": DeskaDbUser()},
        {"timestamp": CurrentTimestamp(), "commitMessage": "commit-fresh",
         "revision": v2, "author": DeskaDbUser()}
    ]
    r.assertEqual(r.c(listRevisions()), ListEnd(vlist))

    # check filters as well
    r.assertEqual(r.c(listRevisions({"condition": "columnGe", "metadata": "revision",
                   "value": v1})), vlist)
