import datetime
import os

class CurrentTimestamp(object):
    def __init__(self):
        self.start = datetime.datetime.now() - datetime.timedelta(seconds=1)

    def __eq__(self, other):
        if not isinstance(other,str):
            raise TypeError, "Cannot compare CurrentTimestamp with anything but string"
        otherDate = datetime.datetime.strptime(other[:19], "%Y-%m-%d %H:%M:%S")
        end = datetime.datetime.now() + datetime.timedelta(seconds=1)
        return self.start <= otherDate and otherDate <= end

class DeskaDbUser(object):
    def __init__(self):
        self.user = os.environ["DESKA_USER"]

    def __eq__(self, other):
        if not isinstance(other,str):
            raise TypeError, "Cannot compare DeskaDbUser with anything but string"
        return other == self.user

class AnyOrderList(object):
    def __init__(self, items):
        self.items = frozenset(items)

    def __eq__(self, other):
        if not isinstance(other, list):
            raise TypeError, "Cannot compare AnyOrderList with anything but list"
        return self.items == frozenset(other)
