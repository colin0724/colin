cmake_minimum_required(VERSION 2.8.11)

project(CheckMats)

add_library(CheckMats SHARED check_mats.cpp)

install(TARGETS CheckMats DESTINATION ${CMAKE_BINARY_DIR}/lib)
