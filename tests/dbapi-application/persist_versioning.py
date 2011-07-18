from apiUtils import *

vlist = [
        {"timestamp": CurrentTimestamp(), "commitMessage": "commit-resumed",
         "revision": Variable("v1"), "author": DeskaDbUser()},
        {"timestamp": CurrentTimestamp(), "commitMessage": "commit-fresh",
         "revision": Variable("v2"), "author": DeskaDbUser()}
    ]

clist = [
    {"status": "DETACHED", "changeset": Variable("changeset"),
     "author": DeskaDbUser(), "timestamp": CurrentTimestamp(), "parentRevision":
     Any(), "message": "detaching"}
]

declarative = [
    # at first, we can create a changeset
    startChangeset().returns(Any()),
    # but trying that once again will fail
    startChangeset().throws(ChangesetAlreadyOpenError()),
    # so let's abort the real changeset
    abortCurrentChangeset(),
    # we won't be able to abort it twice, though
    abortCurrentChangeset().throws(NoChangesetError()),
    # so now we're outside of a changeset

    # try to commit, outside of a changeset
    commitChangeset("commit-outside-a-changeset").throws(NoChangesetError()),

    # try to commit a resumed changeset
    startChangeset().register("changeset"),
    detachFromCurrentChangeset("detaching"),
    detachFromCurrentChangeset("detaching once again").throws(NoChangesetError()),
    # FIXME: resumeChangeset("xyz").throws(ChangesetParsingError()),
    resumeChangeset("xyz").throws(ServerError()),
    pendingChangesets().returns(ListEnd(clist)),
    # FIXME: call pendingChangesets with a filter
    resumeChangeset(Variable("changeset")),
    resumeChangeset(Variable("changeset")).throws(ChangesetAlreadyOpenError()),
    commitChangeset("commit-resumed").register("v1"),

    # try to commit a fresh one
    startChangeset().register("v2"),
    commitChangeset("commit-fresh").register("v2"),
    listRevisions().returns(ListEnd(vlist)),

    # check filters as well
    listRevisions({"condition": "columnGe", "metadata": "revision",
                   "value": Variable("v1")}).returns(vlist),

]
