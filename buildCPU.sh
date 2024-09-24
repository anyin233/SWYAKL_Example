#!/bin/bash

PROJECT_ROOT=$(pwd)/$(dirname $BASH_SOURCE)

source /usr/sw/swgcc/setenv-release-SEA-1432
source /usr/sw/mpi/setenv-mpi-swuc

cmake -S${PROJECT_ROOT} -B${PROJECT_ROOT}/build_mpe \
	-DYAKL_ARCH="" \
	-DYAKL_CXX_FLAGS="-g -O3" \
	-DYAKL_F90_FLAGS="-g -O3" \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DYAKL_PROFILE=ON \
	--toolchain ${PROJECT_ROOT}/cmake/toolchains/sw64mpi.cmake

cmake --build ${PROJECT_ROOT}/build_mpe -j12