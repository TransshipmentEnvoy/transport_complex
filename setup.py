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
from setup_ext import path_util

sys.path.pop(0)

# current packages
packages = [
    "tcomplex",
]

# clib & ext
import nanobind

libraries = [
    cmake_clib.CMakeClib(
        "dep_spdlog",
        sourcedir=str(here / "src" / "buildsys" / "dep" / "spdlog"),
        targetdir=path_util.PathPrefixBuildTemp("prefix/spdlog"),
        cmake_configure_argdef={},
    ),
    cmake_clib.CMakeClib(
        "dep_range-v3",
        sourcedir=str(here / "src" / "buildsys" / "dep" / "range-v3"),
        targetdir=path_util.PathPrefixBuildTemp("prefix/range-v3"),
        cmake_configure_argdef={},
    ),
    cmake_clib.CMakeClib(
        "libtcomplex",
        sourcedir=str(here / "src" / "libtcomplex"),
        targetdir=path_util.PathPrefixBuildLib("tcomplex"),
        cmake_configure_argdef={
            "spdlog_ROOT": path_util.PathPrefixBuildTemp("prefix/spdlog"),
            "range-v3_ROOT": path_util.PathPrefixBuildTemp("prefix/range-v3"),
        },
    ),
]
ext_modules = [
    cmake_extension.CMakeExtension(
        "tcomplex._if",
        sourcedir=str(here / "src" / "tcomplex_if"),
        cmake_configure_argdef={
            "nanobind_ROOT": nanobind.cmake_dir(),
            "libtcomplex_ROOT": path_util.PathPrefixBuildLib("tcomplex/libtcomplex"),
        },
        extra_lib=["libnanobind.so", "nanobind.dll"],
    )
]

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
