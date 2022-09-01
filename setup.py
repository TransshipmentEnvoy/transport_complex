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
from setup_ext import setuptools_wrap, meta_build, cmake_clib, cmake_extension

sys.path.pop(0)

# current packages
packages = [
    "tcomplex",
]

# clib & ext
import dep_spdlog
import nanobind

libraries = [
    cmake_clib.CMakeClib(
        "libtcomplex",
        sourcedir=str(here / "src" / "libtcomplex"),
        targetdir="tcomplex",
    ),
]
ext_modules = []

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
        "build_clib": meta_build.MetaBuildClib,
        "build_ext": meta_build.MetaBuildExt,
    },
    libraries=libraries,
    ext_modules=ext_modules,
)
