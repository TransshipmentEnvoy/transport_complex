cmake_minimum_required(VERSION 3.17...3.24)

# project
project(libtcomplex)

include(GNUInstallDirs)
set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}) # explicitly set DOCDIR location each time

find_package(spdlog REQUIRED)
message(STATUS "use spdlog: ${spdlog_DIR}")

add_library(tcomplex SHARED)
add_library(libtcomplex::tcomplex ALIAS tcomplex)
set_target_properties(tcomplex PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(tcomplex PROPERTIES CXX_STANDARD 20)
set_target_properties(tcomplex PROPERTIES CXX_EXTENSION OFF)
target_sources(tcomplex PRIVATE
    src/lib.cpp
    include/libtcomplex/lib.h)
target_link_libraries(tcomplex PUBLIC 
    spdlog::spdlog)
target_include_directories(tcomplex PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
set_target_properties(tcomplex PROPERTIES
    INSTALL_RPATH "\$ORIGIN")

# install
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS tcomplex
    EXPORT libtcomplex
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT libtcomplex
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libtcomplex
    NAMESPACE libtcomplex::
    FILE libtcomplex-config.cmake)