'''High-level Python bindings for accessing the Deska database

This module provides a convenient way of accessing various objects from the
Deska database as native Python classes.  A connection to the database is
constructed according to the default rules (inheriting necessary options from
the environment and configuration files where applicable); this means that this
module can only be used when the database is reachable and available.
'''
try:
    import libLowLevelPyDeska as _l
except ImportError:
    # a hack for RHEL6 and its paths
    import sys
    sys.path.append("/usr/lib64/python2.6/site-packages/deska")
    import libLowLevelPyDeska as _l
from classes import _discoverScheme, _addExpressionOperators

def init(conn=None):
    _kinds = {}
    if conn is None:
        import os
        if os.environ.has_key("DESKA_VIA_FD_R") and os.environ.has_key("DESKA_VIA_FD_W"):
            rfd = int(os.environ["DESKA_VIA_FD_R"])
            wfd = int(os.environ["DESKA_VIA_FD_W"])
            if rfd < 0 or wfd < 0:
                raise ValueError("DESKA_VIA_FD_R or DESKA_VIA_FD_W are invalid")
            conn = _l.Connection(rfd, wfd)
        else:
            raise RuntimeError("Cannot determine how to access the Deska database. "
                               "Please pass along the connection object or set DESKA_VIA_FD_R and DESKA_VIA_FD_W.")
    _discoverScheme(conn, _kinds)
    _addExpressionOperators()

    for k, v in _kinds.iteritems():
        globals()[k] = v
