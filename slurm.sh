#!/bin/bash -l
#SBATCH -D ./
#SBATCH --export=ALL

#SBATCH -J BENCHRUNS

#SBATCH -o OUTPUT.%N.%j
#SBATCH -e ERROR.%N.%j

#SBATCH -N 1
#SBATCH -p gpu-l40s
#SBATCH --gres=gpu:1
#SBATCH -n 6

#SBATCH -t 10

module load nvhpc

export LD_LIBRARY_PATH=/opt/apps/pkg/compilers/cuda/12.6.2/targets/x86_64-linux/lib/:$LD_LIBRARY_PATH

if [ ! -d "CLBlast" ]; then
    git clone https://github.com/CNugteren/CLBlast
    cd CLBlast
    mkdir build install
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=../install ..
    make -j"$(nproc)"
    make install
    cd ../../
fi

export LD_LIBRARY_PATH=./CLBlast/install/lib64/:$LD_LIBRARY_PATH

make examples/ bench/ EXTRA_INC_DIRS=./CLBlast/install/include/

./examples/info

# This work made use of the Barkla High Performance Computing facilities at the University of Liverpool.
