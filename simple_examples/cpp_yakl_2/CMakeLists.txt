cmake_minimum_required(VERSION 3.25)
project(YAKL_Porting
        LANGUAGES CXX Fortran)  #指定编程语言
        
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${YAKL_ARCH}" STREQUAL "CUDA")
  enable_language(CUDA)
endif() #cuda相关

add_subdirectory("YAKL")  #在当前目录下面放一个YAKL
include(YAKL/yakl_utils.cmake)
add_executable(main main.cpp) #需要能够寻址到main.cpp以及生成可执行文件
yakl_process_target(main)