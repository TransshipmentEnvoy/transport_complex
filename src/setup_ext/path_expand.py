import os
from typing import MutableMapping


class PathPrefixBuildLib:
    def __init__(self, offset="") -> None:
        self.offset = os.path.normcase(os.path.normpath(offset))


class PathPrefixBuildTemp:
    def __init__(self, offset="") -> None:
        self.offset = os.path.normcase(os.path.normpath(offset))


SCAN_LIST = (
    PathPrefixBuildLib,
    PathPrefixBuildTemp,
)


def expand_prefix_argdef(argdef: MutableMapping[str, str], buildlibdir: str, buildtempdir: str):
    for k in argdef:
        v = argdef[k]
        if isinstance(v, SCAN_LIST):
            argdef[k] = expand_path(v, buildlibdir, buildtempdir)


def expand_path(v, buildlibdir: str, buildtempdir: str):
    if isinstance(v, PathPrefixBuildLib):
        offset = v.offset
        new_v = os.path.normcase(os.path.normpath(os.path.abspath(os.path.join(buildlibdir, offset))))
        return new_v
    elif isinstance(v, PathPrefixBuildTemp):
        offset = v.offset
        new_v = os.path.normcase(os.path.normpath(os.path.abspath(os.path.join(buildtempdir, offset))))
        return new_v
