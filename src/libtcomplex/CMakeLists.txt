cmake_minimum_required(VERSION 3.17...3.24)

project(libtcomplex)

find_package(spdlog REQUIRED)

add_library(tcomplex SHARED
    lib.cpp)
set_target_properties(tcomplex PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(tcomplex PROPERTIES CXX_STANDARD 20)
set_target_properties(tcomplex PROPERTIES CXX_EXTENSION OFF)
target_link_directories(tcomplex PRIVATE 
    spdlog::spdlog)

install(TARGETS tcomplex LIBRARY DESTINATION .)