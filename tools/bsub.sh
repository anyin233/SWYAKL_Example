#!/bin/bash
exec_name=$(basename $1)

# export GATOR_DISABLE="T"
bsub -I -b -o ${exec_name}.log -q q_sw_expr -n 1 -cgsp 64 -share_size 13120 -cross_size 1024 -cache_size 128  $1
