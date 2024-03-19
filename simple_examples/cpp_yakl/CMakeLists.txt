cmake_minimum_required(VERSION 3.21)
project(Simple_Serial_Example)


add_subdirectory("YAKL")
include(YAKL/yakl_utils.cmake)

add_executable(main main.cpp)
yakl_process_target(main)