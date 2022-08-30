from setuptools import setup

import os
import sys
import pathlib
import importlib.util


# setup util function
def module_from_file(module_name, file_location):
    spec = importlib.util.spec_from_file_location(module_name, file_location)
    assert spec is not None, f"failed to load module {module_name} at {file_location}"
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None, f"ModuleSpec.loader is None for {module_name} at {file_location}"
    spec.loader.exec_module(module)
    return module


# get setup.py location
here = pathlib.Path(__file__).parent.resolve()
long_description = (here / "README.md").read_text(encoding="utf-8")

# import from location
setup_ext = module_from_file("setup_ext", here / "src" / "setup_ext" / "__init__.py")
sys.modules["setup_ext"] = setup_ext
setuptools_wrap = module_from_file("setup_ext.setuptools_wrap", here / "src" / "setup_ext" / "setuptools_wrap.py")
meta_build = module_from_file("setup_ext.meta_build", here / "src" / "setup_ext" / "meta_build.py")
cmake_extension = module_from_file("setup_ext.cmake_extension", here / "src" / "setup_ext" / "cmake_extension.py")


# current packages
packages = [
    "tcomplex",
]


# ext
import nanobind

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
        "build_ext": meta_build.MetaBuildExt,
    },
    ext_modules=[],
)
