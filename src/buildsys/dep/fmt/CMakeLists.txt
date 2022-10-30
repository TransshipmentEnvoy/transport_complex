cmake_minimum_required(VERSION 3.17...3.24)

include(ExternalProject)

# project
project(dep_fmt)

# option
option(DEP_FMT_VERSION "dep_fmt use version")

if(NOT DEP_FMT_VERSION)
    set(DEP_FMT_VERSION "9.1.0")
endif()

message(STATUS "[fmt] use version: ${DEP_FMT_VERSION}")

ExternalProject_Add(fmt
    GIT_REPOSITORY    "https://github.com/fmtlib/fmt.git"
    GIT_TAG           "${DEP_FMT_VERSION}"
    CMAKE_ARGS        "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
                      "-DCMAKE_CXX_STANDARD=23"
                      "-DFMT_INSTALL=ON"
                      "-DFMT_DOC=OFF"
                      "-DFMT_TEST=OFF"
                      "-DFMT_OS=ON"
                      "-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE"
)
