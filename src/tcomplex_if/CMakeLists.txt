cmake_minimum_required(VERSION 3.17...3.24)

project(tcomplex_if)

find_package(libtcomplex REQUIRED)
find_package(spdlog REQUIRED)

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

install(TARGETS nanobind LIBRARY DESTINATION .)
install(TARGETS _if LIBRARY DESTINATION .)
# a little bit stupid, but this is the best I can find out of cmake
install(IMPORTED_RUNTIME_ARTIFACTS libtcomplex::tcomplex
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .)
install(IMPORTED_RUNTIME_ARTIFACTS spdlog::spdlog
    RUNTIME DESTINATION . NAMELINK_SKIP
    LIBRARY DESTINATION . NAMELINK_SKIP)