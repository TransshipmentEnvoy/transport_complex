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
if (DEFINED DESIGNATED_RPATH)
    message(STATUS "use designated rpath ${DESIGNATED_RPATH}")
    set_target_properties(_if PROPERTIES
        INSTALL_RPATH "${DESIGNATED_RPATH}")
else()
    set_target_properties(_if PROPERTIES
        INSTALL_RPATH "\$ORIGIN")
endif()

if (DEFINED NANOBIND_OFFSET)
    install(TARGETS nanobind 
        RUNTIME DESTINATION "${NANOBIND_OFFSET}"
        LIBRARY DESTINATION "${NANOBIND_OFFSET}")
else()
    install(TARGETS nanobind 
        RUNTIME DESTINATION .
        LIBRARY DESTINATION .)
endif()
install(TARGETS _if LIBRARY DESTINATION .)
