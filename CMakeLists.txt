cmake_minimum_required(VERSION 3.25)
project(YAKL_Porting
        LANGUAGES CXX Fortran)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(YAKL_ARCH SW)

# set(CMAKE_CXX_COMPILER "/usr/sw/swgcc/swgcc710-tools-SEA-1417/usr/bin/swg++")
if ("${YAKL_ARCH}" STREQUAL "CUDA")
  enable_language(CUDA)
endif()


add_subdirectory(YAKL)
include(${PROJECT_SOURCE_DIR}/YAKL/yakl_utils.cmake)
add_subdirectory(src)
