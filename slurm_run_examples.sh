#!/bin/bash -l
#SBATCH -D ./
#SBATCH --export=ALL

#SBATCH -J EXAMPLES

#SBATCH -o OUTPUT.%j.%a
#SBATCH -e ERROR.%j.%a

#SBATCH -N 1
#SBATCH -p gpu-l40s
#SBATCH --gres=gpu:1
#SBATCH -n 6

#SBATCH -t 10

EXAMPLES=$(find examples/ -type f -executable)

CL_PATH=/opt/apps/pkg/compilers/cuda/12.6.2/targets/x86_64-linux
export LD_LIBRARY_PATH=$CL_PATH/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=$CL_PATH/lib:$LIBRARY_PATH
export C_INCLUDE_PATH=$CL_PATH/include:$C_INCLUDE_PATH

for example in $EXAMPLES; do
    $example
done

# This work made use of the Barkla High Performance Computing facilities at the University of Liverpool.
