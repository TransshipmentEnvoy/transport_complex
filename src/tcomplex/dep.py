import os
import sys

import dep_spdlog


def prepend_path(env, path_str):
    path_str = os.path.normcase(os.path.normpath(os.path.abspath(path_str)))
    if sys.platform == "linux":
        if "LD_LIBRARY_PATH" not in env:
            env["LD_LIBRARY_PATH"] = path_str
        else:
            ld_library_path_list = env["LD_LIBRARY_PATH"].split(os.pathsep)
            if path_str not in ld_library_path_list:
                env["LD_LIBRARY_PATH"] = path_str + os.pathsep + env["LD_LIBRARY_PATH"]
    else:
        raise NotImplementedError("unsupported platform")


def calc_libtcomplex_path(here):
    for offset in ["lib64", "lib"]:
        libtcomplex_path = os.path.join(here, "libtcomplex", offset)
        if os.path.exists(libtcomplex_path):
            return libtcomplex_path
    else:
        raise RuntimeError("cannot determine libtcomplex path!")


def setup_env(env=None):
    if env is None:
        env = os.environ

    here = os.path.dirname(os.path.normpath(__file__))

    dep_spdlog.setup_env(env)

    libtcomplex_path = calc_libtcomplex_path(here)
    prepend_path(env, libtcomplex_path)

    prepend_path(env, here)
