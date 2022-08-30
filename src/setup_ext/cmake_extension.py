import os
import re
import subprocess
import sys
from typing import Optional, MutableMapping, Any
from setuptools import Extension

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(
        self,
        name: str,
        sourcedir: str = "",
        cmake_generator: Optional[str] = None,
        cmake_configure_argdef: Optional[MutableMapping[str, str]] = None,
        cmake_build_argdef: Optional[MutableMapping[str, str]] = None,
    ):
        Extension.__init__(self, name, sources=[])

        self.sourcedir = os.path.abspath(sourcedir)

        self.cmake_generator = cmake_generator
        if cmake_configure_argdef is None:
            cmake_configure_argdef = {}
        self.cmake_configure_argdef = cmake_configure_argdef
        if cmake_build_argdef is None:
            cmake_build_argdef = {}
        self.cmake_build_argdef = cmake_build_argdef


def build_extension(
    ext: CMakeExtension,
    extdir: str,
    build_temp: str,
    compiler: Any,
    debug: Any,
    plat_name: Any,
    parallel: Optional[Any],
):
    # required for auto-detection & inclusion of auxiliary "native" libs
    if not extdir.endswith(os.path.sep):
        extdir += os.path.sep

    # initialize the cmake_arg and build_arg
    cmake_arg, build_arg, install_arg = [], [], []

    # determine build type. use explicit setting first, then check self.debug
    if "CMAKE_BUILD_TYPE" not in ext.cmake_configure_argdef:
        if debug:
            ext.cmake_configure_argdef["CMAKE_BUILD_TYPE"] = "Debug"
        else:
            ext.cmake_configure_argdef["CMAKE_BUILD_TYPE"] = "Release"
    build_type = ext.cmake_configure_argdef["CMAKE_BUILD_TYPE"]

    # determine generator (and platform if msvc)
    if compiler.compiler_type != "msvc":
        # Using Ninja-build since it a) is available as a wheel and b)
        # multithreads automatically. MSVC would require all variables be
        # exported for Ninja to pick it up, which is a little tricky to do.
        # Users can override the generator with CMAKE_GENERATOR in CMake
        # 3.15+.
        if ext.cmake_generator is None:
            try:
                import ninja  # noqa: F401

                ext.cmake_generator = "Ninja"
            except ImportError:
                pass
    else:
        # Single config generators are handled "normally"
        single_config = (
            any(x in ext.cmake_generator for x in {"NMake", "Ninja"}) if ext.cmake_generator is not None else False
        )

        # CMake allows an arch-in-generator style for backward compatibility
        contains_arch = (
            any(x in ext.cmake_generator for x in {"ARM", "Win64"}) if ext.cmake_generator is not None else False
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
    if ext.cmake_generator is not None:
        cmake_arg += ["-G", ext.cmake_generator]

    # OSX?
    if sys.platform.startswith("darwin"):
        # Cross-compile support for macOS - respect ARCHFLAGS if set
        archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
        if archs:
            ext.cmake_configure_argdef["CMAKE_OSX_ARCHITECTURES"] = ";".join(archs)

    # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
    # across all generators.
    if "CMAKE_BUILD_PARALLEL_LEVEL" not in ext.cmake_build_argdef:
        # self.parallel is a Python 3 only way to set parallel jobs by hand
        # using -j in the build_ext call, not supported by pip or PyPA-build.
        if parallel:
            # CMake 3.12+ only.
            build_arg += [f"-j{parallel}"]
    else:
        build_arg += [f"-j{ext.cmake_build_argdef['CMAKE_BUILD_PARALLEL_LEVEL']}"]

    # install
    ext.cmake_configure_argdef["CMAKE_INSTALL_PREFIX"] = extdir

    # conclude configure and build argdef
    for arg_key, arg_value in ext.cmake_configure_argdef.items():
        cmake_arg += [f"-D{arg_key}={arg_value}"]

    # create build dir
    build_temp = os.path.join(build_temp, ext.name)
    if os.path.exists(build_temp):
        if os.path.exists(os.path.join(build_temp, "CMakeCache.txt")):
            os.remove(
                os.path.join(build_temp, "CMakeCache.txt")
            )  # the cached environment has gone, so remove the cache
    else:
        os.makedirs(build_temp)

    subprocess.check_call(["cmake", ext.sourcedir] + cmake_arg, cwd=build_temp)
    subprocess.check_call(["cmake", "--build", "."] + build_arg, cwd=build_temp)
    subprocess.check_call(["cmake", "--install", "."] + install_arg, cwd=build_temp)
