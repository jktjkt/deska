import sys
sys.path.append(".")
import libLowLevelPyDeska as _l

class _Kind(object):
    """Container storing _AttributePlaceholder s"""

    def __init__(self, kind):
        """Simply remember the kind name for later"""
        self.kind = kind

    def __getattribute__(self, name):
        """Provide a nice exception instead of a generic one"""
        try:
            return object.__getattribute__(self, name)
        except AttributeError:
            raise AttributeError, "Kind '%s' does not have attribute '%s'" % \
                (self.kind, name)

    def __repr__(self):
        return "%s(%s; attributes: %s)" % (self.__class__, self.kind,
            [attr for attr in dir(self) if not attr.startswith("__")] )


class _AttributePlaceholder(object):
    """Represent an object's attribute in a filter statement"""

    def __init__(self, kind, attribute):
        if not isinstance(kind, str):
            raise TypeError, "kind has to be string, not %s" % type(kind)
        if not isinstance(attribute, str):
            raise TypeError, "attribute has to be string, not %s" % type(attribute)
        self.kind = kind
        self.attribute = attribute

    def __repr__(self):
        return "%s(%s.%s)" % (self.__class__, self.kind, self.attribute)

    def __eq__(self, other):
        return _l.AttributeExpression(
            _l.ComparisonOperator.COLUMN_EQ, self.kind, self.attribute,
            _l.Py_2_DeskaDbValue(other))

    def __ne__(self, other):
        return _l.AttributeExpression(
            _l.ComparisonOperator.COLUMN_NE, self.kind, self.attribute,
            _l.Py_2_DeskaDbValue(other))


def _discoverScheme(conn, target=None):
    for kind in conn.kindNames():
        # Create a placeholder object at the kind level; this will contain all attributes
        kindInstance = _Kind(kind)
        for attr in conn.kindAttributes(kind):
            # create our attribute placeholders
            setattr(kindInstance, attr.name,
                    _AttributePlaceholder(kind, attr.name))
        # and register the completed object
        if target is not None:
            target[kind] = kindInstance
        else:
            globals()[kind] = kindInstance

def _op_AndFilter_and(self, other):
    if isinstance(other, _l.AndFilter):
        return _l.AndFilter(self.operands + other.operands)
    elif isinstance(other, (_l.AttributeExpression, _l.MetadataExpression)):
        res = self.operands
        res.append(_l.Filter(_l.Expression(other)))
        return _l.AndFilter(res)
    else:
        raise NotImplementedError

def _op_Expression_and(self, other):
    if isinstance(other, (_l.AttributeExpression, _l.MetadataExpression)):
        res = _l.std_vector_Filter()
        res.append(_l.Filter(_l.Expression(self)))
        res.append(_l.Filter(_l.Expression(other)))
        return _l.AndFilter(res)
    elif isinstance(other, _l.AndFilter):
        return _l.AndFilter([self] + other.operands)
    else:
        raise NotImplementedError

def _addExpressionOperators():
    """Add overloaded operators to the expression classes"""
    _l.AttributeExpression.__and__ = _op_Expression_and
    _l.AndFilter.__and__ = _op_AndFilter_and

if __name__ == "__main__":
    conn = _l.Connection()
    _discoverScheme(conn)
    _addExpressionOperators()
    #print dict(globals())
    #print host
    #print vendor

    print hardware.vendor == "ahoj"
    #print host.note == "bleh"
    #print host.pwn == "bleh"
    op1 = hardware.vendor == "ahoj"
    op2 = host.note == "foo"
    op3 = host.note != None
    print op1 & op2
    print op1 & op2 & op3
