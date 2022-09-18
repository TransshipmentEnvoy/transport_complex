cmake_minimum_required(VERSION 3.17...3.24)

include(ExternalProject)

# project
project(dep_robin-map)

# option
option(DEP_ROBIN-MAP_VERSION "dep_robin-map use version")

if(NOT DEP_ROBIN-MAP_VERSION)
    set(DEP_ROBIN-MAP_VERSION "v1.0.1")
endif()

message(STATUS "[robin-map] use version: ${DEP_ROBIN-MAP_VERSION}")

ExternalProject_Add(robin-map
    GIT_REPOSITORY    "https://github.com/Tessil/robin-map.git"
    GIT_TAG           "${DEP_ROBIN-MAP_VERSION}"
    CMAKE_ARGS        "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
)
