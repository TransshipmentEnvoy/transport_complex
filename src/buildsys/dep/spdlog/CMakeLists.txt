cmake_minimum_required(VERSION 3.17...3.24)


# project
project(dep_spdlog)

# option
option(DEP_SPDLOG_VERSION "dep_spdlog use version")
option(DEP_SPDLOG_FETCHCONTENT "dep_spdlog use fetchcontent (also disable soname)")

if(NOT DEP_SPDLOG_VERSION)
    set(DEP_SPDLOG_VERSION "v1.10.0")
endif()

message(STATUS "[spdlog] use version: ${DEP_SPDLOG_VERSION}")

if(NOT DEP_SPDLOG_FETCHCONTENT)
    include(ExternalProject)
    message(STATUS "[spdlog] use ExternalProject")
    ExternalProject_Add(spdlog
        GIT_REPOSITORY    "git@github.com:gabime/spdlog.git"
        GIT_TAG           "${DEP_SPDLOG_VERSION}"
        CMAKE_ARGS        "-DSPDLOG_BUILD_SHARED:BOOL=ON"
                          "-DSPDLOG_FMT_EXTERNAL:BOOL=ON"
                          "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
                          "-Dfmt_ROOT=${fmt_ROOT}"
    )
else()
    set(FETCHCONTENT_QUIET FALSE)
    include(FetchContent)
    message(STATUS "[spdlog] use FetchContent")
    FetchContent_Declare(spdlog
        GIT_REPOSITORY    "git@github.com:gabime/spdlog.git"
        GIT_TAG           "${DEP_SPDLOG_VERSION}"
    )

    message(STATUS "[spdlog] disable soname")
    set(SPDLOG_BUILD_SHARED ON CACHE INTERNAL "")
    set(SPDLOG_FMT_EXTERNAL ON CACHE INTERNAL "")
    set(SPDLOG_INSTALL ON CACHE INTERNAL "")
    set(fmt_ROOT ${fmt_ROOT} CACHE INTERNAL "")
    FetchContent_MakeAvailable(spdlog)
    set_target_properties(spdlog PROPERTIES NO_SONAME True)
endif()

