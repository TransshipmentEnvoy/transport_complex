import os
import re
import subprocess
import shlex
import sys
from typing import Optional, MutableMapping, Any

import logging

_logger_cmake_clib = logging.getLogger("setup_ext.CmakeClib")

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


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
    compiler: Any,
    debug: Any,
    plat_name: Any,
    parallel: Optional[Any],
):
    _logger_cmake_clib.info("build cmake clib: %s >>>", clib.name)

    # required for auto-detection & inclusion of auxiliary "native" libs
    if not clibdir.endswith(os.path.sep):
        clibdir += os.path.sep

    # initialize the cmake_arg and build_arg
    cmake_arg, build_arg, install_arg = [], [], []

    # determine build type. use explicit setting first, then check self.debug
    if "CMAKE_BUILD_TYPE" not in clib.cmake_configure_argdef:
        if debug:
            clib.cmake_configure_argdef["CMAKE_BUILD_TYPE"] = "Debug"
        else:
            clib.cmake_configure_argdef["CMAKE_BUILD_TYPE"] = "Release"
    build_type = clib.cmake_configure_argdef["CMAKE_BUILD_TYPE"]

    # determine generator (and platform if msvc)
    if compiler.compiler_type != "msvc":
        # Using Ninja-build since it a) is available as a wheel and b)
        # multithreads automatically. MSVC would require all variables be
        # exported for Ninja to pick it up, which is a little tricky to do.
        # Users can override the generator with CMAKE_GENERATOR in CMake
        # 3.15+.
        if clib.cmake_generator is None:
            try:
                import ninja  # noqa: F401

                clib.cmake_generator = "Ninja"
            except ImportError:
                pass
    else:
        # Single config generators are handled "normally"
        single_config = (
            any(x in clib.cmake_generator for x in {"NMake", "Ninja"}) if clib.cmake_generator is not None else False
        )

        # CMake allows an arch-in-generator style for backward compatibility
        contains_arch = (
            any(x in clib.cmake_generator for x in {"ARM", "Win64"}) if clib.cmake_generator is not None else False
        )

        # Specify the arch if using MSVC generator, but only if it doesn't
        # contain a backward-compatibility arch spec already in the
        # generator name.
        if not single_config and not contains_arch:
            cmake_arg += ["-A", PLAT_TO_CMAKE[plat_name]]

        # Multi-config generators have a different way to specify configs
        if not single_config:
            build_arg += ["--config", build_type]
            install_arg += ["--config", build_type]
    if clib.cmake_generator is not None:
        cmake_arg += ["-G", clib.cmake_generator]

    # OSX?
    if sys.platform.startswith("darwin"):
        # Cross-compile support for macOS - respect ARCHFLAGS if set
        archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
        if archs:
            clib.cmake_configure_argdef["CMAKE_OSX_ARCHITECTURES"] = ";".join(archs)

    # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
    # across all generators.
    if "CMAKE_BUILD_PARALLEL_LEVEL" not in clib.cmake_build_argdef:
        # self.parallel is a Python 3 only way to set parallel jobs by hand
        # using -j in the build_clib call, not supported by pip or PyPA-build.
        if parallel:
            # CMake 3.12+ only.
            build_arg += [f"-j{parallel}"]
    else:
        build_arg += [f"-j{clib.cmake_build_argdef['CMAKE_BUILD_PARALLEL_LEVEL']}"]

    # install
    clib.cmake_configure_argdef["CMAKE_INSTALL_PREFIX"] = clibdir

    # conclude configure and build argdef
    for arg_key, arg_value in clib.cmake_configure_argdef.items():
        cmake_arg += [f"-D{arg_key}={arg_value}"]

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
