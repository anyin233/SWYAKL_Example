#!/bin/bash

bin_path=$(dirname $0)/build/bin/stencil

for ((i=64; i<=8192; i*=2))
do
  echo "Running stencil with $i^2 points"
  output=$(./tools/bsub-allshare.sh $bin_path $i $i)
  # save output into log/$i.log
  echo $output > log/$i.log
done