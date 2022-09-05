cmake_minimum_required(VERSION 3.17...3.24)

include(ExternalProject)

# project
project(dep_spdlog)

# option
option(DEP_SPDLOG_VERSION "dep_spdlog use version")

if(NOT DEP_SPDLOG_VERSION)
    set(DEP_SPDLOG_VERSION "v1.10.0")
endif()

message(STATUS "[spdlog] use version: ${DEP_SPDLOG_VERSION}")

ExternalProject_Add(spdlog
    GIT_REPOSITORY    "git@github.com:gabime/spdlog.git"
    GIT_TAG           "${DEP_SPDLOG_VERSION}"
    CMAKE_ARGS        "-DSPDLOG_BUILD_SHARED:BOOL=ON"
                      "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
)
