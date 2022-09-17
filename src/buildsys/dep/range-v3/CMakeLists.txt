cmake_minimum_required(VERSION 3.17...3.24)

include(ExternalProject)

# project
project(dep_range-v3)

# option
option(DEP_RANGE-V3_VERSION "dep_range-v3 use version")

if(NOT DEP_RANGE-V3_VERSION)
    set(DEP_RANGE-V3_VERSION "0.12.0")
endif()

message(STATUS "[range-v3] use version: ${DEP_RANGE-V3_VERSION}")

ExternalProject_Add(range-v3
    GIT_REPOSITORY    "https://github.com/ericniebler/range-v3.git"
    GIT_TAG           "${DEP_RANGE-V3_VERSION}"
    CMAKE_ARGS        "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
                      "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                      "-DRANGES_CXX_STD=23"
                      "-DRANGES_BUILD_CALENDAR_EXAMPLE=OFF"
                      "-DRANGE_V3_TESTS=OFF"
                      "-DRANGE_V3_EXAMPLES=OFF"
                      "-DRANGE_V3_DOCS=OFF"
)
