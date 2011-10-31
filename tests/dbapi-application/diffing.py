from apiUtils import *

def helper_extract_commands(list):
    """Extract commands from a list of ApiMethod instances and filter out the "tag" values from each dict on the fly"""
    return [dict([(k,v) for (k,v) in y.iteritems() if k != "tag"]) for y in [x.command for x in list]]

def helper_diff_2_cmdlist(diff):
    newlist = []
    for cmd in diff:
        if cmd["command"] == "setAttribute":
            del cmd["oldValue"]
            cmd["attributeData"] = cmd["newValue"]
            del cmd["newValue"]
        newlist.append(cmd)
    return newlist

def revert_cmdlist(list):
    res = []
    for item in list:
        cmd = item.command
        if cmd["command"] == "setAttribute":
            cmd["attributeData"] = cmd["oldValue"]
            if cmd["attributeData"] == 'None':
                cmd["attributeData"] = None
            del cmd["oldValue"]
            del cmd["newValue"]
        elif cmd["command"] == "createObject":
            cmd["command"] = "deleteObject"
        elif cmd["command"] == "deleteObject":
            cmd["command"] = "createObject"
        res.prepend(cmd)
    return res

def imperative(r):
    changeset = r.c(startChangeset())
    revA = r.c(pendingChangesets(filter={"condition": "columnEq", "metadata":
                                         "changeset", "value": changeset})
              )[0]["parentRevision"]

    cmdlist1 = [
        createObject("vendor", "v1"),
        createObject("vendor", "v2"),
        createObject("hardware", "hw1")
    ]
    cmdlist2 = [
        # do NOT reshuffle this list!
        setAttribute("hardware", "hw1", "vendor", "v1"),
        setAttribute("hardware", "hw1", "purchase", "2011-01-01"),
        setAttribute("hardware", "hw1", "warranty", "2011-01-01")
    ]
    for x in cmdlist1:
        r.c(x)
    for x in cmdlist2:
        r.cvoid(x)

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revB = r.c(commitChangeset("test diff"))
    r.assertTrue(revA < revB)

    reportedDiff = r.c(dataDifference(revA, revB))

    r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                   sorted(helper_extract_commands(cmdlist1 + cmdlist2)))
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    changeset = r.c(startChangeset())
    cmdlist3 = [
        setAttribute("hardware", "hw1", "vendor", "v2"),
    ]
    for x in cmdlist3:
        r.cvoid(x)

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revC = r.c(commitChangeset("changed one attribute"))
    reportedDiff = r.c(dataDifference(revB, revC))
    r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                   sorted(helper_extract_commands(cmdlist3)))
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    reportedDiff = r.c(dataDifference(revA, revC))
    # now be careful, we have to filter out the modification to hw1.vendor
    r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                   sorted(helper_extract_commands(cmdlist1 + cmdlist2[1:] + cmdlist3)))

    # try renaming an object
    changeset = r.c(startChangeset())
    cmdlist4 = [ renameObject("vendor", "v1", "v1a") ]
    for x in cmdlist4:
        r.cvoid(x)
    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))
    revD = r.c(commitChangeset("renaming stuff"))
    reportedDiff = r.c(dataDifference(revC, revD))
    # FIXME: redmine #286, renames are reported back as setAttribute calls :(
    #r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
    #               sorted(helper_extract_commands(cmdlist4)))
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    # now let's remove what we've added
    changeset = r.c(startChangeset())
    cmdlist4 = [
        deleteObject("vendor", "v2"),
        deleteObject("hardware", "hw1"),
        deleteObject("vendor", "v1a"),
    ]
    for x in cmdlist4:
        r.cvoid(x)

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revE = r.c(commitChangeset("removed stuff"))
    # try with both of them
    for rev in (revB, revC):
        reportedDiff = r.c(dataDifference(rev, revE))
        r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                       sorted(helper_extract_commands([
                           deleteObject("vendor", "v2"),
                           deleteObject("hardware", "hw1"),
                           # this is different, we got to use the old name, not
                           # the new one
                           deleteObject("vendor", "v1"),
                       ])))
        # FIXME: redmine #277
        #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    reportedDiff = r.c(dataDifference(revD, revE))
    r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                   sorted(helper_extract_commands(cmdlist4)))
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    # finally, there should be absolutely no difference in here
    # FIXME: Redmine #284, this one is broken
    #r.assertEquals(r.c(dataDifference(revA, revE)), [])
