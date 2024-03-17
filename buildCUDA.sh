#!/bin/bash

# Set the path to the CUDA toolkit
export PATH=$PATH:/usr/local/cuda-11.7/bin

cmake -DYAKL_ARCH="CUDA" \
	-DYAKL_CUDA_FLAGS="-arch=sm_70 -g -O3" \
	-DYAKL_F90_FLAGS="-g -O3" \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DYAKL_PROFILE=ON \
	..
