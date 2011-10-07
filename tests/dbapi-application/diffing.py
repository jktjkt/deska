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

    # FIXME: redmine #283
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revB = r.c(commitChangeset("test diff"))
    r.assertTrue(revA < revB)

    reportedDiff = r.c(dataDifference(revA, revB))

    r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                   sorted(helper_extract_commands(cmdlist1 + cmdlist2)))
    # FIXME: redmine #283
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    changeset = r.c(startChangeset())
    cmdlist3 = [
        setAttribute("hardware", "hw1", "vendor", "v2"),
    ]
    for x in cmdlist3:
        r.cvoid(x)

    # FIXME: redmine #283
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revC = r.c(commitChangeset("changed one attribute"))
    reportedDiff = r.c(dataDifference(revB, revC))
    r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                   sorted(helper_extract_commands(cmdlist3)))
    # FIXME: redmine #283
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    reportedDiff = r.c(dataDifference(revA, revC))
    # now be careful, we have to filter out the modification to hw1.vendor
    r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
                   sorted(helper_extract_commands(cmdlist1 + cmdlist2[1:] + cmdlist3)))



def foo():
    def test_001_dataDifference(self):
        # commit
        res = self.command(js.commitChangeset,"test diff")
        self.OK(res.OK)
        revB = res.result()
        revA = testutils.updateRev(revB,-1)

        # diff is ok?
        res = self.command(js.dataDifference,revA,revB)
        self.OK(res.OK)
        diff = res.result()
        listToCompare = self.diff2cmdlist(diff)
        self.assertEqual(listToCompare.sort(),cmdlist.sort())

        # and undo changes
        # start changeset
        res = self.command(js.startChangeset)
        self.OK(res.OK)
        # revert diff and run
        cmdlist = self.revertDiff(diff)
        self.commandList(cmdlist)

        # commit
        res = self.command(js.commitChangeset,"test diff")
        self.OK(res.OK)
        revC = res.result()

        # diff should be empty
        res = self.command(js.dataDifference,revA,revC)
        self.OK(res.OK)
        self.assertTrue(res.result() == [])

        # diff is ok?
        res = self.command(js.dataDifference,revB,revC)
        self.OK(res.OK)
        diff = res.result()
        listToCompare = self.diff2cmdlist(diff)
        self.assertEqual(listToCompare.sort(),cmdlist.sort())

    def test_002_dataDifferenceTemp(self):
        '''test diff in changeset'''
        # start changeset
        res = self.command(js.startChangeset)
        self.OK(res.OK)
        cmdlist = list()
        # add commands into cmdlist and run
        cmdlist.append(json.loads(js.createObject("vendor","test vendor")))
        cmdlist.append(json.loads(js.createObject("vendor","test vendor 2")))
        cmdlist.append(json.loads(js.createObject("hardware","test hardware 1")))
        cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","vendor","test vendor")))
        cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","purchase","2011-01-01")))
        cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","warranty","2011-01-01")))
        self.commandList(cmdlist)

        # diff is ok?
        res = self.command(js.dataDifferenceInTemporaryChangeset)
        self.OK(res.OK)
        diff = res.result()
        listToCompare = self.diff2cmdlist(diff)
        self.assertEqual(listToCompare.sort(),cmdlist.sort())

        # abort
        res = self.command(js.abortCurrentChangeset)
