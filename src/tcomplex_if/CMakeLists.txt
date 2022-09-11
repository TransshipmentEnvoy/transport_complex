cmake_minimum_required(VERSION 3.17...3.24)

project(tcomplex_if)

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
    _if
    NB_SHARED
    bind.cpp)
target_link_libraries(_if PRIVATE libtcomplex::tcomplex)
set_target_properties(_if PROPERTIES
    INSTALL_RPATH "\$ORIGIN")

install(TARGETS nanobind 
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .)
install(TARGETS _if LIBRARY DESTINATION .)
