#!/bin/bash

bin_path=$(dirname $0)/build/bin/stencil_l

# clear old logs
rm -rf log/*

for ((i=6; i<=13; i+=1))
do
  n=$((2**$i))
  n3d=$((40+70*($i-6)))
  echo "Running stencil with $n*$n points, and $n3d*$n3d*$n3d points in 3D Kernels"
  output=$(./tools/bsub.sh $bin_path $n $n3d)
  # save output into log/$i.log
  # echo $output > log/$i.log
done

# Extract time information
./extract_time.sh -i log -o output