# Fortran源文件
# 设置Fortran编译标志

set(MOD_OUTPUT_DIR ${CMAKE_BINARY_DIR}/mod)

add_library(slave_fortran_lib OBJECT fortran_module.f90)
set_target_properties(slave_fortran_lib PROPERTIES
    COMPILE_FLAGS "-mslave -msimd -mieee"
    Fortran_MODULE_DIRECTORY ${MOD_OUTPUT_DIR}/slave_fortran_lib
)

add_library(fortran_lib OBJECT fortran_module.f90)
set_target_properties(fortran_lib PROPERTIES
    COMPILE_FLAGS "-mhost -msimd -mieee"
    Fortran_MODULE_DIRECTORY ${MOD_OUTPUT_DIR}/fortran_lib
)


yakl_process_target(fortran_lib)
yakl_process_target(slave_fortran_lib)
# C++可执行文件
add_executable(call_from_fortran main.cpp)
# 链接Fortran库
target_link_libraries(call_from_fortran fortran_lib slave_fortran_lib)
yakl_process_target(call_from_fortran)
