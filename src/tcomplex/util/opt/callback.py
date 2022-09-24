import typing

from .type_def import OptEntryValueUnspecified


class OptEntryCallback:
    dependency: list[str] = []
    always: bool = False

    def __call__(self, curr_key: str, curr_value: typing.Any, prog: str, dep: typing.Mapping) -> typing.Any:
        return OptEntryValueUnspecified


def resolve_callback_dependency(callback_map: typing.Mapping[str, OptEntryCallback]):
    # obtain adj list
    all_keys = set(callback_map.keys())
    adj_list = {}
    for k, v in callback_map.items():
        dlist = []
        for d in v.dependency:
            if d in all_keys:
                dlist.append(d)
        adj_list[k] = dlist

    # get proc requirement
    proc_dep_list = adj_list.copy()
    proc_clear_list: dict[str, list[str]] = {}
    for k, dlist in proc_dep_list.items():
        for vv in dlist:
            if vv not in proc_clear_list:
                proc_clear_list[vv] = []
            proc_clear_list[vv].append(k)

    # toposort
    res = []
    while True:
        if len(proc_dep_list) == 0:
            break

        # iterate over proc_dep_list to find whether have a 0
        k_curr = next(iter(proc_dep_list.keys()))
        indeg_curr = len(proc_dep_list[k_curr])
        for _k in proc_dep_list:
            _indeg = len(proc_dep_list[_k])
            if _indeg < indeg_curr:
                k_curr = _k
                indeg_curr = _indeg
                if indeg_curr == 0:
                    break

        if indeg_curr > 0:
            return None

        res.append(k_curr)

        if k_curr in proc_clear_list:
            clear_list = proc_clear_list[k_curr]
            for k_node in clear_list:
                proc_dep_list[k_node].remove(k_curr)
            proc_clear_list.pop(k_curr)

        proc_dep_list.pop(k_curr)
    return res
