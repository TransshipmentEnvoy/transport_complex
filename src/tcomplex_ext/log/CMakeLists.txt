cmake_minimum_required(VERSION 3.17...3.24)

project(tcomplex_ext_log)

find_package(libtcomplex REQUIRED)

# python
if(MSVC)
    find_package(PythonInterp)
    find_package(Python)
endif()
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# nanobind
find_package(nanobind REQUIRED)

# nanobind extension
nanobind_add_module(
    _log
    NB_SHARED
    log.cpp)
target_link_libraries(_log PRIVATE libtcomplex::tcomplex)
if (DEFINED DESIGNATED_RPATH)
    message(STATUS "use designated rpath ${DESIGNATED_RPATH}")
    set_target_properties(_log PROPERTIES
        INSTALL_RPATH "${DESIGNATED_RPATH}")
else()
    set_target_properties(_log PROPERTIES
        INSTALL_RPATH "\$ORIGIN")
endif()

install(TARGETS _log LIBRARY DESTINATION .)
