from setuptools.command.build_clib import build_clib
from setuptools.command.build_ext import build_ext
from distutils.errors import DistutilsSetupError
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
                pass
            else:
                libraries_.append(lib_item)

        return super().check_library_list(libraries_)

    def get_source_files(self):
        if self.libraries is None:
            return []

        self.check_library_list(self.libraries)

        filenames = []
        for lib_item in self.libraries:
            if isinstance(lib_item, cmake_clib.CMakeClib):
                pass  # TODO: append source
            else:
                (lib_name, build_info) = lib_item
                sources = build_info.get("sources")
                if sources is None or not isinstance(sources, (list, tuple)):
                    raise DistutilsSetupError(
                        "in 'libraries' option (library '%s'), "
                        "'sources' must be present and must be "
                        "a list of source filenames" % lib_name
                    )

                filenames.extend(sources)
        return filenames

    def get_library_names(self):
        if self.libraries is None:
            return None

        lib_names = []
        for lib_item in self.libraries:
            if isinstance(lib_item, cmake_clib.CMakeClib):
                pass
            else:
                (lib_name, build_info) = lib_item
                lib_names.append(lib_name)
        return lib_names

    def build_libraries(self, libraries) -> None:
        libraries_ = []
        for lib_item in libraries:
            if isinstance(lib_item, cmake_clib.CMakeClib):
                _logger_clib.info("detect cmake clib: %s at %s", lib_item.name, lib_item.sourcedir)

                # get whether is inplace build
                build_ext = self.get_finalized_command("build_ext")
                inplace = build_ext.inplace or build_ext.editable_mode

                # get libdir (default is under package root)
                if not inplace:
                    build_lib = os.path.abspath(build_ext.build_lib)
                    lib_dir = os.path.normcase(
                        os.path.normpath(os.path.abspath(os.path.join(build_ext.build_lib, lib_item.targetdir)))
                    )
                else:
                    build_py = self.get_finalized_command("build_py")
                    build_lib = os.path.abspath(build_py.get_package_dir(""))
                    lib_dir = os.path.normcase(
                        os.path.normpath(os.path.abspath(os.path.join(build_lib, lib_item.targetdir)))
                    )

                build_temp = os.path.abspath(os.path.join(self.build_temp, "cmake_clib"))
                _logger_clib.info("build cmake clib: %s at %s", lib_item.name, build_temp)
                _logger_clib.info("build cmake clib: %s to %s", lib_item.name, lib_dir)
                cmake_clib.build_clib(
                    lib_item,
                    lib_dir,
                    build_temp=build_temp,
                    build_lib=build_lib,
                    compiler=self.compiler,
                    debug=self.debug,
                    plat_name=build_ext.plat_name,
                    parallel=build_ext.parallel,
                )
            else:
                libraries_.append(lib_item)

        return super().build_libraries(libraries_)


# extend cmdclasses
class MetaBuildExt(build_ext):
    def build_extension(self, ext) -> None:
        if isinstance(ext, cmake_extension.CMakeExtension):
            extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

            inplace = self.inplace or self.editable_mode
            if not inplace:
                build_lib = self.build_lib
            else:
                build_py = self.get_finalized_command("build_py")
                build_lib = os.path.abspath(build_py.get_package_dir(""))

            cmake_extension.build_extension(
                ext=ext,
                extdir=extdir,
                build_temp=self.build_temp,
                build_lib=build_lib,
                compiler=self.compiler,
                debug=self.debug,
                plat_name=self.plat_name,
                parallel=self.parallel,
            )
        else:
            # fallback to default
            super().build_extension(ext)
