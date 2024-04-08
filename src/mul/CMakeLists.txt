# cmake_minimum_required(VERSION 3.10)
# project(YAKL_test)

include_directories(${PROJECT_SOURCE_DIR}/YAKL)

# add_library(YAKL_test_cpe cpe_adapter.cpp)
# yakl_process_target(YAKL_test_cpe)
# target_compile_options(YAKL_test_cpe PRIVATE -mslave -msimd -mieee)

add_executable(mul main.cpp)
yakl_process_target(mul)

target_link_libraries(mul ${YAKL_TARGET})

# target_include_directories()
