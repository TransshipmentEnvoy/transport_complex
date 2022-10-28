from setuptools import setup

import os
import sys
import pathlib
import importlib.util


# get setup.py location
here = pathlib.Path(__file__).parent.resolve()
long_description = (here / "README.md").read_text(encoding="utf-8")

# import from location
sys.path.insert(0, str(here / "src"))
import setup_ext
from setup_ext import setuptools_wrap, meta_build
from setup_ext import cmake_clib, cmake_extension, cmake_if
from setup_ext import develop_warp
from setup_ext import path_util, rpath_util

sys.path.pop(0)

# current packages
packages = [
    "tcomplex",
]

# clib & ext
import nanobind

# region ====== clib >>>
libraries = [
    cmake_clib.CMakeClib(
        "dep_spdlog",
        sourcedir=str(here / "src" / "buildsys" / "dep" / "spdlog"),
        targetdir=path_util.PathPrefixBuildTemp("prefix/spdlog"),
        cmake_configure_argdef={
            "DEP_SPDLOG_FETCHCONTENT": "ON",
        },
    ),
    cmake_clib.CMakeClib(
        "dep_range-v3",
        sourcedir=str(here / "src" / "buildsys" / "dep" / "range-v3"),
        targetdir=path_util.PathPrefixBuildTemp("prefix/range-v3"),
        cmake_configure_argdef={},
    ),
    cmake_clib.CMakeClib(
        "dep_fmt",
        sourcedir=str(here / "src" / "buildsys" / "dep" / "fmt"),
        targetdir=path_util.PathPrefixBuildTemp("prefix/fmt"),
        cmake_configure_argdef={},
    ),
    cmake_clib.CMakeClib(
        "dep_robin-map",
        sourcedir=str(here / "src" / "buildsys" / "dep" / "robin-map"),
        targetdir=path_util.PathPrefixBuildTemp("prefix/robin-map"),
        cmake_configure_argdef={},
    ),
]
# main lib
MAIN_LIB_TARGET = "tcomplex/lib"
main_lib = cmake_clib.CMakeClib(
    "libtcomplex",
    sourcedir=str(here / "src" / "libtcomplex"),
    targetdir=path_util.PathPrefixBuildLib(MAIN_LIB_TARGET),
    cmake_configure_argdef={
        "spdlog_ROOT": path_util.PathPrefixBuildTemp("prefix/spdlog"),
        "range-v3_ROOT": path_util.PathPrefixBuildTemp("prefix/range-v3"),
        "fmt_ROOT": path_util.PathPrefixBuildTemp("prefix/fmt"),
        "tsl-robin-map_ROOT": path_util.PathPrefixBuildTemp("prefix/robin-map"),
    },
)
libraries.append(main_lib)
# endregion === clib <<<

# region ====== ext >>>
ext_modules = [
    cmake_extension.CMakeExtension(
        "tcomplex._if",
        sourcedir=str(here / "src" / "tcomplex_ext" / "if"),
        cmake_configure_argdef={
            "nanobind_ROOT": nanobind.cmake_dir(),
            "libtcomplex_ROOT": path_util.PathPrefixBuildLib("tcomplex/lib/libtcomplex"),
            "DESIGNATED_RPATH": rpath_util.compute_rpath("tcomplex._if", MAIN_LIB_TARGET),
            "NANOBIND_OFFSET": rpath_util.compute_relpath("tcomplex._if", MAIN_LIB_TARGET),
        },
        extra_lib={
            "libnanobind.so": rpath_util.compute_relpath("tcomplex._if", MAIN_LIB_TARGET),
            "nanobind.dll": rpath_util.compute_relpath("tcomplex._if", MAIN_LIB_TARGET),
        },
    ),
    cmake_extension.CMakeExtension(
        "tcomplex.util.upkeep._log",
        sourcedir=str(here / "src" / "tcomplex_ext" / "log"),
        cmake_configure_argdef={
            "nanobind_ROOT": nanobind.cmake_dir(),
            "libtcomplex_ROOT": path_util.PathPrefixBuildLib("tcomplex/lib/libtcomplex"),
            "DESIGNATED_RPATH": rpath_util.compute_rpath("tcomplex.util.upkeep._log", MAIN_LIB_TARGET),
        },
    ),
]
# endregion === ext <<<

setuptools_wrap.setup(
    name="tcomplex",
    version="0.0.0",
    description="Transport Complex",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/TransshipmentEnvoy/transport_complex",
    author="TransshipmentEnvoy",
    author_email="TransshipmentEnvoy@outlook.com",
    # dep
    python_requires=">=3.9, <4",
    # pkg
    package_dir={"": "src"},
    packages=packages,
    # cmdclass
    cmdclass={
        "develop": develop_warp.CustomDevelop,
        "build_clib": meta_build.MetaBuildClib,
        "build_ext": meta_build.MetaBuildExt,
    },
    libraries=libraries,
    ext_modules=ext_modules,
)
