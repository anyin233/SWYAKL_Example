add_executable(Fortran_YAKL main.f90)
add_library(CPP_Lib cpp_interface.cpp)
yakl_process_target(Fortran_YAKL)
yakl_process_target(CPP_Lib)

set_target_properties(Fortran_YAKL PROPERTIES LINKER_LANGUAGE Fortran)
target_link_libraries(Fortran_YAKL CPP_Lib ${YAKL_TARGET} ${YAKL_FORTRAN_INTERFACE_TARGET})
