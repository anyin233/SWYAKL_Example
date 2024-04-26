#!/bin/bash
exec_name=$(basename $1)

# export GATOR_DISABLE="T"
bsub -I -b -o log/${exec_name}_${2}.log -q q_sw_expr -n 1 -cgsp 64 -share_size 15000 -cross_size 1024 -cache_size 128 $@
