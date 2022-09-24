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
    log
    NB_SHARED
    log.cpp)
target_link_libraries(log PRIVATE libtcomplex::tcomplex)
set_target_properties(log PROPERTIES
    INSTALL_RPATH "\$ORIGIN")

install(TARGETS log LIBRARY DESTINATION .)
