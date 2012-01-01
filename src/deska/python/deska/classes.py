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
        val = _l.DeskaDbValue_2_Py(x.data())
        if isinstance(val, _l.std_set_std_string):
            val = [item for item in val]
        setattr(res, x.key(), val)
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
            condition = _l.OptionalFilter(_l.Filter(_l.Expression(condition)))
        elif isinstance(condition, (_l.Expression, _l.AndFilter, _l.OrFilter)):
            condition = _l.OptionalFilter(_l.Filter(condition))
        elif isinstance(condition, _l.Filter):
            condition = _l.OptionalFilter(condition)
        elif isinstance(condition, _l.OptionalFilter):
            pass
        else:
            raise TypeError, "Object filtering expects a proper Filter, not %s" % type(condition)

        if not self.conn:
            raise ValueError, "No active session"

        ret_map = self.conn.multipleResolvedObjectData(self.kind, condition)
        return dict((x.key(), _map_to_class_with_values(x.data())) for x in ret_map)

    def _all(self):
        """Implement iteration over all items"""
        return self.__getitem__(_l.OptionalFilter())

class _AttributePlaceholder(object):
    """Represent an object's attribute in a filter statement"""

    def __init__(self, kind, attribute, dataType):
        if not isinstance(kind, str):
            raise TypeError, "kind has to be string, not %s" % type(kind)
        if not isinstance(attribute, str):
            raise TypeError, "attribute has to be string, not %s" % type(attribute)
        self.kind = kind
        self.attribute = attribute
        self.dataType = dataType

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

    def __contains__(self, other):
        if self.dataType != _l.AttributeType.IDENTIFIER_SET:
            raise TypeError, "Column %s is not iterable" % self.name
        else:
            # __contains__ shall return boolean, or at least something directly convertible to boolean.
            # This is unusable here, because we try hard to postpone the decision to the DB side of the wire.
            # I guess that's the similar reason to why the SQL Alchemy also use their own methods here.
            raise NotImplementedError, "Sorry, Python doesn't fully support __contains__ overrides. Use contains() instead and see source for details."

    def contains(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_CONTAINS)

    def notContains(self, other):
        return self._operatorHelper(other, _l.ComparisonOperator.COLUMN_NOT_CONTAINS)


def _discoverScheme(conn, target=None):
    for kind in conn.kindNames():
        # Create a placeholder object at the kind level; this will contain all attributes
        kindInstance = _Kind(kind, conn)
        for attr in conn.kindAttributes(kind):
            if attr.name == "name":
                raise KeyError, "The DB scheme claims that there's a 'name' attribute. That attribute is a special one and should not be returned by DBAPI."
            # create our attribute placeholders
            setattr(kindInstance, attr.name,
                    _AttributePlaceholder(kind, attr.name, attr.type))
        kindInstance.name = _AttributePlaceholder(kind, "name", _l.AttributeType.IDENTIFIER)
        # and register the completed object
        if target is not None:
            target[kind] = kindInstance
        else:
            globals()[kind] = kindInstance

def _convert_to_Filter(foo):
    if isinstance(foo, (_l.AttributeExpression, _l.MetadataExpression)):
        return _l.Filter(_l.Expression(foo))
    elif isinstance(foo, (_l.Expression, _l.OrFilter, _l.AndFilter)):
        return _l.Filter(foo)
    elif isinstance(foo, _l.Filter):
        return foo
    else:
        raise NotImplementedError("Don't know how to convert %s to _l.Filter" %
                                  type(foo))


def _op_AndFilter_and(self, other):
    if isinstance(other, _l.AndFilter):
        return _l.AndFilter(self.operands + other.operands)
    else:
        res = self.operands
        res.append(_convert_to_Filter(other))
        return _l.AndFilter(res)

def _op_generic_and(self, other):
    if isinstance(other, _l.AndFilter):
        return other.__or__(self)
    else:
        res = _l.std_vector_Filter()
        res.append(_convert_to_Filter(self))
        res.append(_convert_to_Filter(other))
        return _l.AndFilter(res)

def _op_generic_or(self, other):
    if isinstance(other, _l.OrFilter):
        return other.__or__(self)
    else:
        res = _l.std_vector_Filter()
        res.append(_convert_to_Filter(self))
        res.append(_convert_to_Filter(other))
        return _l.OrFilter(res)

def _op_OrFilter_or(self, other):
    if isinstance(other, _l.OrFilter):
        return _l.OrFilter(self.operands + other.operands)
    else:
        res = self.operands
        res.append(_convert_to_Filter(other))
        return _l.AndFilter(res)

def _op_nonzero(self):
    """Fake operator for catching bool promotion

    Python cannot provide overrides for the 'and' operator, so we just throw an error here. See PEP 335 for details."""
    raise NotImplementedError, "It does not make sense to cast filter elements to bool. Use '&' instead of 'and'."

def _addExpressionOperators():
    """Add overloaded operators to the expression classes"""
    _l.AttributeExpression.__and__ = _op_generic_and
    _l.AttributeExpression.__or__ = _op_generic_or
    _l.AndFilter.__and__ = _op_AndFilter_and
    _l.AndFilter.__or__ = _op_generic_or
    _l.OrFilter.__or__ = _op_OrFilter_or
    _l.OrFilter.__and__ = _op_generic_and
    _l.AttributeExpression.__nonzero__ = _op_nonzero
    _l.MetadataExpression.__nonzero__ = _op_nonzero
    _l.Expression.__nonzero__ = _op_nonzero
    _l.AndFilter.__nonzero__ = _op_nonzero
    _l.OrFilter.__nonzero__ = _op_nonzero
