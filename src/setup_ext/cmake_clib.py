import os
import re
import subprocess
import shlex
import sys
from typing import Optional, MutableMapping, Any

from .cmake_if import parse_config

import logging

_logger_cmake_clib = logging.getLogger("setup_ext.CmakeClib")


def _log_subprocess_output(pipe):
    for line in iter(pipe.readline, b""):  # b'\n'-separated lines
        _logger_cmake_clib.info("\t%s", line.decode("utf-8").rstrip("\n"))


class CMakeClib:
    def __init__(
        self,
        name: str,
        sourcedir: str = "",
        targetdir: str = "",
        cmake_generator: Optional[str] = None,
        cmake_configure_argdef: Optional[MutableMapping[str, str]] = None,
        cmake_build_argdef: Optional[MutableMapping[str, str]] = None,
    ) -> None:
        self.name = name
        self.sourcedir = os.path.normcase(os.path.normpath(os.path.abspath(sourcedir)))
        self.targetdir = os.path.normcase(os.path.normpath(targetdir))  # could be abs/rel

        self.cmake_generator = cmake_generator
        if cmake_configure_argdef is None:
            cmake_configure_argdef = {}
        self.cmake_configure_argdef = cmake_configure_argdef
        if cmake_build_argdef is None:
            cmake_build_argdef = {}
        self.cmake_build_argdef = cmake_build_argdef


def build_clib(
    clib: CMakeClib,
    clibdir: str,
    build_temp: str,
    build_lib: str,
    compiler: Any,
    debug: Any,
    plat_name: Any,
    parallel: Optional[Any],
):
    _logger_cmake_clib.info("build cmake clib: %s >>>", clib.name)

    cmake_arg, build_arg, install_arg = parse_config(
        installdir=clibdir,
        buildlibdir=build_lib,
        cmake_generator=clib.cmake_generator,
        cmake_configure_argdef=clib.cmake_configure_argdef,
        cmake_build_argdef=clib.cmake_build_argdef,
        compiler=compiler,
        debug=debug,
        plat_name=plat_name,
        parallel=parallel,
    )

    # create build dir
    build_temp = os.path.join(build_temp, clib.name)
    if os.path.exists(build_temp):
        if os.path.exists(os.path.join(build_temp, "CMakeCache.txt")):
            os.remove(
                os.path.join(build_temp, "CMakeCache.txt")
            )  # the cached environment has gone, so remove the cache
    else:
        os.makedirs(build_temp)

    _logger_cmake_clib.info("> working dir: %s", build_temp)

    _logger_cmake_clib.info("> configure: %s", shlex.join(["cmake", clib.sourcedir] + cmake_arg))
    configure_process = subprocess.Popen(
        ["cmake", clib.sourcedir] + cmake_arg, cwd=build_temp, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    with configure_process.stdout:
        _log_subprocess_output(configure_process.stdout)
    configure_process.wait()

    _logger_cmake_clib.info("> build: %s", shlex.join(["cmake", "--build", "."] + build_arg))
    build_process = subprocess.Popen(
        ["cmake", "--build", "."] + build_arg, cwd=build_temp, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    with build_process.stdout:
        _log_subprocess_output(build_process.stdout)
    build_process.wait()

    _logger_cmake_clib.info("> install: %s", shlex.join(["cmake", "--install", "."] + install_arg))
    install_process = subprocess.Popen(
        ["cmake", "--install", "."] + install_arg, cwd=build_temp, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    with install_process.stdout:
        _log_subprocess_output(install_process.stdout)
    install_process.wait()

    _logger_cmake_clib.info("conclude cmake clib: %s ===", clib.name)
