#!/bin/bash

PROJECT_ROOT=$(pwd)/$(dirname $BASH_SOURCE)

# currently swuc has been merged into new version of swgcc
# source /usr/sw/swgcc/setenv-release-SEA-swuc
source /usr/sw/swgcc/setenv-release-SEA-swuc
# source /usr/sw/swgcc/setenv-release-SEA-1417
source /usr/sw/mpi/setenv-mpi-swuc


#
# 1. Generate building script
#

## Intel
# cmake -S. -Bobjdir \
#     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Sunway-Debug
#cmake -S${PROJECT_ROOT} -B${PROJECT_ROOT}/objdir-dbg-2 \
#    -DUSE_SWCPE=ON \
#    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
#    -DCMAKE_TOOLCHAIN_FILE=${PROJECT_ROOT}/cmake/Platform/Toolchain-Sunway.cmake

# Sunway-Debug
cmake -S${PROJECT_ROOT} -B${PROJECT_ROOT}/build \
        -DCMAKE_BUILD_TYPE=Debug \
        -DYAKL_ARCH="SW" \
        -DYAKL_PROFILE=True \
        -DCMAKE_C_COMPILER_WORKS=True \
        -DCMAKE_CXX_COMPILER_WORKS=True \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        --toolchain ${PROJECT_ROOT}/cmake/toolchains/sw64mpi.cmake


# Sunway-Release
# cmake -G Ninja -S${PROJECT_ROOT} -B${PROJECT_ROOT}/build \
#         -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
#         --toolchain ${PROJECT_ROOT}/cmake/toolchains/sw64.cmake

#
# 2. Build 3dsph
#
# (cd objdir-rel; make -j12)