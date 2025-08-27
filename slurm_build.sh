#!/bin/bash -l
#SBATCH -D ./
#SBATCH --export=ALL

#SBATCH -J BUILD

#SBATCH -o OUTPUT.%j.%a
#SBATCH -e ERROR.%j.%a

#SBATCH -N 1
#SBATCH -p gpu-l40s
#SBATCH --gres=gpu:1
#SBATCH -n 6

#SBATCH -t 10

module load nvhpc

CL_PATH=/opt/apps/pkg/compilers/cuda/12.6.2/targets/x86_64-linux

export LD_LIBRARY_PATH=$CL_PATH/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=$CL_PATH/lib:$LIBRARY_PATH
export C_INCLUDE_PATH=$CL_PATH/include:$C_INCLUDE_PATH
INC_FLAGS=$(echo "$C_INCLUDE_PATH" | tr ':' ' ')

# library benchs
make bench/ examples/ EXTRA_INC_DIRS="$INC_FLAGS"

if [ ! -d "CLBlast" ]; then
    git clone https://github.com/CNugteren/CLBlast
    cd CLBlast || exit
    mkdir build install
    cd build || exit
    cmake -DCMAKE_INSTALL_PREFIX=../install -DOPENCL_ROOT=$CL_PATH ..
    make -j"$(nproc)"
    make install
    cd ../../
fi

export LD_LIBRARY_PATH=./CLBlast/install/lib64:$LD_LIBRARY_PATH
export LIBRARY_PATH=./CLBlast/install/lib64:$LIBRARY_PATH
export C_INCLUDE_PATH=./CLBlast/install/include:$C_INCLUDE_PATH
INC_FLAGS=$(echo "$C_INCLUDE_PATH" | tr ':' ' ')
# CLBlast benchs
make benchCLBlast/ EXTRA_INC_DIRS="$INC_FLAGS" EXTRA_LD_LIBS=-lclblast

# OpenACC benchs
for bench in benchOpenACC/*; do
    nvc -acc -O3 -fast -o "${bench%.c}" "$bench"
done

./examples/info

# This work made use of the Barkla High Performance Computing facilities at the University of Liverpool.
