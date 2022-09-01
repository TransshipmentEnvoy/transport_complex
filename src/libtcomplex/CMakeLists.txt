cmake_minimum_required(VERSION 3.17...3.24)

project(libtcomplex)

add_library(tcomplex SHARED
    lib.cpp)

install(TARGETS tcomplex LIBRARY DESTINATION .)