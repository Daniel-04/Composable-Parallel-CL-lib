#!/bin/bash
#!/bin/bash

OUTPUT_FILE="slurm_run_bench.sh"
BENCHS=$(find bench/ benchCLBlast/ benchOpenACC/ -type f -executable)

cat <<EOF >$OUTPUT_FILE
#!/bin/bash -l
#SBATCH -D ./
#SBATCH --export=ALL

#SBATCH -J BENCHRUNS

#SBATCH -o OUTPUT.%j.%a
#SBATCH -e ERROR.%j.%a

#SBATCH -N 1
#SBATCH -p gpu-l40s
#SBATCH --gres=gpu:1
#SBATCH -n 6

#SBATCH -t 10

#SBATCH --array=1-$(echo "$BENCHS" | wc -l)

BENCHMARKS=(
EOF

while IFS= read -r exec; do
    echo "    \"$exec\"" >>$OUTPUT_FILE
done <<<"$BENCHS"

cat <<EOF >>$OUTPUT_FILE

)

ARRAY_INDEX=\$((SLURM_ARRAY_TASK_ID - 1))
BENCH_TO_RUN=\${BENCHMARKS[\$ARRAY_INDEX]}

module load nvhpc
CL_PATH=/opt/apps/pkg/compilers/cuda/12.6.2/targets/x86_64-linux
export LD_LIBRARY_PATH=\$CL_PATH/lib:\$LD_LIBRARY_PATH
export LIBRARY_PATH=\$CL_PATH/lib:\$LIBRARY_PATH
export C_INCLUDE_PATH=\$CL_PATH/include:\$C_INCLUDE_PATH
export LD_LIBRARY_PATH=./CLBlast/install/lib64:\$LD_LIBRARY_PATH
export LIBRARY_PATH=./CLBlast/install/lib64:\$LIBRARY_PATH
export C_INCLUDE_PATH=./CLBlast/install/include:\$C_INCLUDE_PATH

i=512
echo Running "\$BENCH_TO_RUN" from "\$ARRAY_INDEX" with "\$i"
while \$BENCH_TO_RUN \$i; do
      i=\$((i*2))
      echo Running "\$BENCH_TO_RUN" from "\$ARRAY_INDEX" with "\$i"
done

# This work made use of the Barkla High Performance Computing facilities at the University of Liverpool.
EOF
