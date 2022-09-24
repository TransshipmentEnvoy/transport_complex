import sys
import shlex
from copy import deepcopy

import logging

_logger = logging.getLogger("__name__")


def arg_to_string(arg):
    res = "{\n"
    for k, v in vars(arg).items():
        res += f"\t{k:<20}: {v}\n"
    res += "}"
    return res


def argdict_to_string(argdict):
    # TODO: support recursive print to depth
    if argdict is None:
        return r"{}"
    res = "{\n"
    for k, v in argdict.items():
        if not isinstance(v, dict):
            res += f"\t{k:<20}: {v}\n"
        else:
            res += f"\t{k:<20}>\n"
            for sub_k, sub_v in v.items():
                res += f"\t  {sub_k:<18}: {sub_v}\n"
            res += "\t<\n"
    res += "}"
    return res


def get_command():
    return shlex.join(deepcopy(sys.argv))
