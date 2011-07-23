import libLowLevelPyDeska as _l
from classes import _discoverScheme

_kinds = {}
_discoverScheme(_l.Connection(), _kinds)
for k, v in _kinds.iteritems():
    globals()[k] = v
__all__ = _kinds.keys()
