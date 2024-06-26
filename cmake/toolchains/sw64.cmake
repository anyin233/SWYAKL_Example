#
# Set the target operating system
#
set(CMAKE_SYSTEM_NAME Linux CACHE STRING "Cross-compiling for Sunway")
set(CMAKE_SYSTEM_PROCESSOR "sw64")
set(SW64 TRUE)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

#
# Set the compiler
#
set(CMAKE_C_COMPILER ${CMAKE_SOURCE_DIR}/tools/swucwrapper)
set(CMAKE_CXX_COMPILER ${CMAKE_SOURCE_DIR}/tools/swucxxwrapper)
set(CMAKE_Fortran_COMPILER swgfortran)

set(CMAKE_CROSSCOMPILING_EMULATOR "${CMAKE_SOURCE_DIR}/tools/bsub.sh")
add_compile_options(-mhybrid-coding -mieee -mftz -msimd)
