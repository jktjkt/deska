'''High-level Python bindings for accessing the Deska database

This module provides a convenient way of accessing various objects from the
Deska database as native Python classes.  A connection to the database is
constructed according to the default rules (inheriting necessary options from
the environment and configuration files where applicable); this means that this
module can only be used when the database is reachable and available.
'''
import libLowLevelPyDeska as _l
from classes import _discoverScheme

def init(conn=None):
    _kinds = {}
    if conn is None:
        conn = _l.Connection()
    _discoverScheme(conn, _kinds)
    for k, v in _kinds.iteritems():
        globals()[k] = v
