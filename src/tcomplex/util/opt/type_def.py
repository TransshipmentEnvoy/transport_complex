import enum
from enum import auto

import typing
from typing import Any

if typing.TYPE_CHECKING:
    import argparse
    from .reg import OptEntryAttr


@enum.unique
class OptEntrySource(enum.IntEnum):
    BUILTIN = 0
    CALLBACK = auto()
    COMMANDLINE_ONLY = auto()
    CONFIG_ONLY = auto()
    COMMANDLINE_OVER_CONFIG = auto()


class OptEntryCommandlinePattern:
    pass


@enum.unique
class OptEntryCommandlineSeqPattern(OptEntryCommandlinePattern, enum.IntEnum):
    COMMA_SEP = 0
    COLON_SEP = auto()
    SEMICOLON_SEP = auto()


@enum.unique
class OptEntryCommandlineMapPattern(OptEntryCommandlinePattern, enum.IntEnum):
    COMMA_SEP_EQUAL = 0
    COLON_SEP_EQUAL = auto()
    SEMICOLON_SEP_EQUAL = auto()
    COMMA_SEP_COLON = auto()
    SEMICOLON_SEP_COLON = auto()


@enum.unique
class OptEntryCommandlineBoolPattern(OptEntryCommandlinePattern, enum.IntEnum):
    SET_TRUE = 0
    SET_FALSE = auto()
    ON_OFF = auto()


class OptEntryValueUnspecified:
    pass


def analyze_type(in_type, seq_type_list, map_type_list):
    def _analyze_type(_in_type, _seq_type_list, _map_type_list):
        if _in_type == Any:
            return {"proc": None, "cast": None, "next": []}

        origin = typing.get_origin(_in_type)
        if origin is None:
            return {"proc": _in_type, "cast": None, "next": []}

        if origin in _seq_type_list:
            args = typing.get_args(_in_type)
            if len(args) != 1:
                raise TypeError(f"type parameter error! got {args} with len {len(args)}")
            argtype = args[0]
            extra_proc = _analyze_type(argtype, _seq_type_list, _map_type_list)
            return {"proc": origin, "cast": "seq", "next": [extra_proc]}

        if origin in _map_type_list:
            args = typing.get_args(_in_type)
            if len(args) not in [1, 2]:
                raise TypeError(f"type parameter error! got {args} with len {len(args)}")
            if len(args) == 1 and args[0] == Any:
                args = (Any, Any)
            keyargtype = args[0]
            key_extra_proc = _analyze_type(keyargtype, _seq_type_list, _map_type_list)
            next_list = [key_extra_proc]
            if len(args) == 2:
                valueargtype = args[1]
                value_extra_proc = _analyze_type(valueargtype, _seq_type_list, _map_type_list)
                next_list.append(value_extra_proc)
            else:
                next_list.append({"proc": None, "cast": None, "next": []})
            return {"proc": origin, "cast": "map", "next": next_list}

        raise TypeError(f"unsupported container type! got {origin}")

    return _analyze_type(in_type, seq_type_list, map_type_list)


def supported_by_commandline(proclist):
    if proclist["cast"] == "seq":
        return len(proclist["next"][0]["next"]) == 0
    if proclist["cast"] == "map":
        return (len(proclist["next"][0]["next"]) == 0) and (len(proclist["next"][1]["next"]) == 0)
    return True


def proclist_pattern_paired(proclist, cmdpattern, supported_seq, supported_map):
    if proclist["cast"] == "seq":
        return isinstance(cmdpattern, OptEntryCommandlineSeqPattern)
    if proclist["cast"] == "map":
        return isinstance(cmdpattern, OptEntryCommandlineMapPattern)
    if proclist["proc"] in supported_seq:
        return isinstance(cmdpattern, OptEntryCommandlineSeqPattern)
    if proclist["proc"] in supported_map:
        return isinstance(cmdpattern, OptEntryCommandlineMapPattern)
    if proclist["proc"] == bool:
        return isinstance(cmdpattern, OptEntryCommandlineBoolPattern)
    return cmdpattern is None


def cast_to_res(blob, proclist):
    def _cast_to_res(_blob, _proclist):
        _res = _blob
        if _proclist["cast"] == "seq":
            _res = list(_blob)
            next_list = _proclist["next"]
            assert len(next_list) == 1
            next_proclist = next_list[0]
            for offset in range(len(_res)):
                _res[offset] = _cast_to_res(_res[offset], next_proclist)
        elif _proclist["cast"] == "map":
            _res = dict(_blob)
            next_list = _proclist["next"]
            assert len(next_list) == 2
            _res2 = _res
            _res = {}
            for k, v in _res2.items():
                new_k = _cast_to_res(k, next_list[0])
                new_v = _cast_to_res(v, next_list[1])
                _res[new_k] = new_v

        if _proclist["proc"] is not None:
            _res = _proclist["proc"](_res)

        return _res

    return _cast_to_res(blob, proclist)


def handle_cmd_seq(in_str, sep_type):
    if sep_type == OptEntryCommandlineSeqPattern.COMMA_SEP:
        sep = ","
    elif sep_type == OptEntryCommandlineSeqPattern.COLON_SEP:
        sep = ":"
    elif sep_type == OptEntryCommandlineSeqPattern.SEMICOLON_SEP:
        sep = ";"
    else:
        raise TypeError(f"unknown cmdline seq pattern! got {sep_type}")

    return in_str.split(sep)


def handle_cmd_map(in_str, sep_type):
    if sep_type == OptEntryCommandlineMapPattern.COMMA_SEP_EQUAL:
        sep_pair, sep_eq = ",", "="
    elif sep_type == OptEntryCommandlineMapPattern.COLON_SEP_EQUAL:
        sep_pair, sep_eq = ":", "="
    elif sep_type == OptEntryCommandlineMapPattern.SEMICOLON_SEP_EQUAL:
        sep_pair, sep_eq = ";", "="
    elif sep_type == OptEntryCommandlineMapPattern.COMMA_SEP_COLON:
        sep_pair, sep_eq = ",", ":"
    elif sep_type == OptEntryCommandlineMapPattern.SEMICOLON_SEP_COLON:
        sep_pair, sep_eq = ";", ":"
    else:
        raise TypeError(f"unknown cmdline map pattern! got {sep_type}")

    base_list = in_str.split(sep_pair)
    res = {}
    for base_pair in base_list:
        k, v = base_pair.split(sep_eq)
        res[k] = v
    return res


def hook_cmd_bool(
    parser: argparse.ArgumentParser, key: str, pattern: OptEntryCommandlineBoolPattern, meta: OptEntryAttr
):
    if pattern == OptEntryCommandlineBoolPattern.SET_TRUE:
        parser.add_argument(f"--{key}", action="store_true", help=meta.desc)
    elif pattern == OptEntryCommandlineBoolPattern.SET_FALSE:
        parser.add_argument(f"--{key}", action="store_false", help=meta.desc)
    elif pattern == OptEntryCommandlineBoolPattern.ON_OFF:
        parser.add_argument(f"--{key}", action="store_true", help=meta.desc)
        opt_str = list(parser._option_string_actions.keys())
        if f"--{key}__off" in opt_str:
            raise KeyError(f"parser already have string action `--{key}__off`!")
        off_desc = meta.desc + " (off)" if meta.desc is not None else None
        parser.add_argument(f"--{key}__off", action="store_true", help=off_desc)
    else:
        raise TypeError(f"unknown cmdline bool pattern! got {pattern}")


def handle_cmd_bool(parse_res: argparse.Namespace, key: str, pattern: OptEntryCommandlineBoolPattern):
    if pattern == OptEntryCommandlineBoolPattern.SET_TRUE:
        res = getattr(parse_res, key)
    elif pattern == OptEntryCommandlineBoolPattern.SET_FALSE:
        res = getattr(parse_res, key)
    elif pattern == OptEntryCommandlineBoolPattern.ON_OFF:
        on_flag = getattr(parse_res, key)
        off_flag = getattr(parse_res, f"{key}__off")
        if not on_flag and not off_flag:
            res = OptEntryValueUnspecified
        elif on_flag and not off_flag:
            res = True
        elif not on_flag and off_flag:
            res = False
        else:
            raise RuntimeError(f"both on and off flag are set! key: {key}")
    else:
        raise TypeError(f"unknown cmdline bool pattern! got {pattern}")

    return res
