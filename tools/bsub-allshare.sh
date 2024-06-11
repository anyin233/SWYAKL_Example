#!/bin/bash
exec_name=$(basename $1)

# export GATOR_DISABLE="T"
bsub -I -b -o log/${exec_name}_${2}.log -q q_sw_expr -n 1 -cgsp 64 -mpecg 6 -cross_size 86000 -share_size 400 -cache_size 128 -xmalloc $@
