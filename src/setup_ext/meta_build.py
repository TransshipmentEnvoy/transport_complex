from setuptools.command.build_clib import build_clib
from setuptools.command.build_ext import build_ext
import os
import logging
from . import cmake_clib, cmake_extension


_logger_clib = logging.getLogger("setup_ext.MetaBuildClib")
# TODO: mata-build-clib
class MetaBuildClib(build_clib):
    def check_library_list(self, libraries):
        if libraries is None:
            return

        libraries_ = []
        for lib_item in libraries:
            if isinstance(lib_item, cmake_clib.CMakeClib):
                _logger_clib.warning("detect cmake clib: %s at %s", lib_item.name, lib_item.sourcedir)
            else:
                libraries_.append(lib_item)

        return super().check_library_list(libraries_)

    def get_source_files(self):
        if self.libraries is None:
            return []
        return super().get_source_files()

    def build_libraries(self, libraries) -> None:
        libraries_ = []
        for lib_item in libraries:
            if isinstance(lib_item, cmake_clib.CMakeClib):
                pass
            else:
                libraries_.append(lib_item)

        return super().build_libraries(libraries_)


# extend cmdclasses
class MetaBuildExt(build_ext):
    def build_extension(self, ext) -> None:
        if isinstance(ext, cmake_extension.CMakeExtension):
            extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
            cmake_extension.build_extension(
                ext=ext,
                extdir=extdir,
                build_temp=self.build_temp,
                compiler=self.compiler,
                debug=self.debug,
                plat_name=self.plat_name,
                parallel=self.parallel,
            )
        else:
            # fallback to default
            super().build_extension(ext)
