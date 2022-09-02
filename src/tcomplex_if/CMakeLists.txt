cmake_minimum_required(VERSION 3.17...3.24)

project(tcomplex_if)

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
    _if
    NB_SHARED
    bind.cpp)

install(TARGETS nanobind LIBRARY DESTINATION .)
install(TARGETS _if LIBRARY DESTINATION .)
