cmake_minimum_required(VERSION 3.17...3.24)

# project
project(libtcomplex)

find_package(spdlog REQUIRED)
message(STATUS "use spdlog: ${spdlog_DIR}")
find_package(range-v3 REQUIRED)
message(STATUS "use range-v3: ${range-v3_DIR}")
find_package(fmt REQUIRED)
message(STATUS "use fmt: ${fmt_DIR}")

add_library(tcomplex SHARED)
add_library(libtcomplex::tcomplex ALIAS tcomplex)
set_target_properties(tcomplex PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(tcomplex PROPERTIES CXX_STANDARD 23)
set_target_properties(tcomplex PROPERTIES CXX_EXTENSION OFF)
target_sources(tcomplex PRIVATE
    src/lib.cpp
    src/log.h
    src/log.cpp
    include/libtcomplex/lib.h)
target_link_libraries(tcomplex PRIVATE 
    spdlog::spdlog
    range-v3::range-v3
    fmt::fmt)
target_include_directories(tcomplex PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/libtcomplex/include>)
set_target_properties(tcomplex PROPERTIES
    INSTALL_RPATH "\$ORIGIN")
# TODO: change symbol visibility in the future
set_target_properties(tcomplex PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# install
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/libtcomplex/include)
install(TARGETS tcomplex
    EXPORT libtcomplex
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/libtcomplex/lib)
install(EXPORT libtcomplex
    DESTINATION ${CMAKE_INSTALL_PREFIX}/libtcomplex/lib/cmake/libtcomplex
    NAMESPACE libtcomplex::
    FILE libtcomplex-config.cmake)

# install dep
install(IMPORTED_RUNTIME_ARTIFACTS spdlog::spdlog
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .)
