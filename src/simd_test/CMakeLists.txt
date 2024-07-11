# cmake_minimum_required(VERSION 3.10)
# project(YAKL_test)

include_directories(${PROJECT_SOURCE_DIR}/YAKL)

# add_library(YAKL_test_cpe cpe_adapter.cpp)
# yakl_process_target(YAKL_test_cpe)
# target_compile_options(YAKL_test_cpe PRIVATE -mslave -msimd -mieee)

add_executable(simd_test main.cpp)
yakl_process_target(simd_test)

target_link_libraries(simd_test ${YAKL_TARGET} -lswperf)
target_link_directories(simd_test PUBLIC /usr/sw/penv/lib)
target_include_directories(simd_test PUBLIC /usr/sw/penv/include)

# target_include_directories()
