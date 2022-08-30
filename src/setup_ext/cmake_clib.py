import os
import re
import subprocess
import sys
from typing import Optional, MutableMapping, Any
from setuptools import Extension


class CMakeClib:
    def __init__(
        self,
        name: str,
        sourcedir: str = "",
        cmake_generator: Optional[str] = None,
        cmake_configure_argdef: Optional[MutableMapping[str, str]] = None,
        cmake_build_argdef: Optional[MutableMapping[str, str]] = None,
    ) -> None:
        self.name = name
        self.sourcedir = os.path.abspath(sourcedir)

        self.cmake_generator = cmake_generator
        if cmake_configure_argdef is None:
            cmake_configure_argdef = {}
        self.cmake_configure_argdef = cmake_configure_argdef
        if cmake_build_argdef is None:
            cmake_build_argdef = {}
        self.cmake_build_argdef = cmake_build_argdef


def build_extension(
    clib: CMakeClib,
    clibdir: str,
    build_temp: str,
    compiler: Any,
    debug: Any,
    plat_name: Any,
    parallel: Optional[Any],
):
    pass
