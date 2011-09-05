import sys
sys.path.append(".")
import libLowLevelPyDeska as _l

class _KindInstanceInResult(object):
    """A structure holding effective values of an object, as retrieved by the objectData"""

    def __init__(self):
        pass

    def __repr__(self):
        return "{%s}" % ", ".join("%s: %s" % (attr, self.__getattribute__(attr)) for attr in dir(self) if not attr.startswith("__"))

def _map_to_class_with_values(d):
    """Convert a map<string, Value> into a class with native Python data"""
    res = _KindInstanceInResult()
    for x in d:
        setattr(res, x.key(), _l.DeskaDbValue_2_Py(x.data()))
    return res

class _Kind(object):
    """Container storing _AttributePlaceholder s"""

    def __init__(self, kind, conn):
        """Simply remember the kind name for later"""
        self.kind = kind
        self.conn = conn

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

    def __getitem__(self, condition):
        """Implementation of the filtering"""
        if isinstance(condition, (_l.AttributeExpression,_l.MetadataExpression)):
            condition = _l.Filter(_l.Expression(condition))
        elif isinstance(condition, (_l.Expression, _l.AndFilter, _l.OrFilter)):
            condition = _l.Filter(condition)
        elif isinstance(condition, _l.Filter):
            pass
        else:
            raise TypeError, "Object filtering expects a proper Filter, not %s" % type(condition)

        if not self.conn:
            raise ValueError, "No active session"

        # FIXME: change this to multipleResolvedObjectData when it gets supported by the server side
        ret_map = self.conn.multipleObjectData(self.kind, condition)
        return dict((x.key(), _map_to_class_with_values(x.data())) for x in ret_map)

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


    def _operatorHelper(self, other, comparator):
        """Helper for __eq__, __ne__, __gt__, __ge__"""
        return _l.AttributeExpression(
            comparator, self.kind, self.attribute, _l.Py_2_DeskaDbValue(other))

    def __eq__(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_EQ)

    def __ne__(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_NE)

    def __lt__(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_LT)

    def __le__(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_LE)

    def __gt__(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_GT)

    def __ge__(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_GE)


def _discoverScheme(conn, target=None):
    for kind in conn.kindNames():
        # Create a placeholder object at the kind level; this will contain all attributes
        kindInstance = _Kind(kind, conn)
        for attr in conn.kindAttributes(kind):
            if attr.name == "name":
                raise KeyError, "The DB scheme claims that there's a 'name' attribute. That attribute is a special one and should not be returned by DBAPI."
            # create our attribute placeholders
            setattr(kindInstance, attr.name,
                    _AttributePlaceholder(kind, attr.name))
        kindInstance.name = _AttributePlaceholder(kind, "name")
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

def _op_nonzero(self):
    """Fake operator for catching bool promotion

    Python cannot provide overrides for the 'and' operator, so we just throw an error here. See PEP 335 for details."""
    raise NotImplementedError, "It does not make sense to cast filter elements to bool. Use '&' instead of 'and'."

def _addExpressionOperators():
    """Add overloaded operators to the expression classes"""
    _l.AttributeExpression.__and__ = _op_Expression_and
    _l.AndFilter.__and__ = _op_AndFilter_and
    _l.AttributeExpression.__nonzero__ = _op_nonzero
    _l.MetadataExpression.__nonzero__ = _op_nonzero
    _l.Expression.__nonzero__ = _op_nonzero
    _l.AndFilter.__nonzero__ = _op_nonzero
    _l.OrFilter.__nonzero__ = _op_nonzero

if __name__ == "__main__":
    conn = _l.Connection()
    _discoverScheme(conn)
    _addExpressionOperators()
    #print dict(globals())
    #print host
    #print vendor

    print hardware.vendor == "ahoj"
    #print host.note_host == "bleh"
    #print host.pwn == "bleh"
    op1 = hardware.vendor == "ahoj"
    op2 = host.note_host == "foo"
    op3 = host.note_host != None
    print op1 & op2
    print op1 & op2 & op3
    print host[_l.AndFilter(_l.std_vector_Filter())]
    # Fails, redmine#266
    #print host[host.hardware != None]
    # Fails (empty result), redmine#264
    print host[host.name != None]
    print host[host.name == "a"]
    print host[host.name == "b"]
    print host[host.note_host == "foo"]
    print hardware[op1]
    #print hardware[op1 & op2 & op3]
