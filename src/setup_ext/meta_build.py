from setuptools.command.build_clib import build_clib
from setuptools.command.build_ext import build_ext
import os
from . import cmake_extension

# TODO: mata-build-clib
class MetaBuildClib(build_clib):
    def check_library_list(self, libraries):
        if libraries is None:
            return
        return super().check_library_list(libraries)

    def get_source_files(self):
        if self.libraries is None:
            return []
        return super().get_source_files()

    def run(self):
        res = super().run()
        return res


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
