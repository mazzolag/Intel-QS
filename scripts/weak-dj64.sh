#!/bin/bash -l

# this part will be ignored if:
# - nodes are preallocated with salloc
# - and the file is executed as just run.sh (not sbatch run.sh)
#SBATCH --nodes=64
#SBATCH --constraint=mc
#SBATCH --partition=normal
#SBATCH --time=120

# loading necessary modules
echo "============================="
echo "  LOADING MODULES"
echo "============================="
module load daint-mc
module swap PrgEnv-cray PrgEnv-intel
module load intel
module unload cray-libsci

export CC=`which cc`
export CXX=`which CC`
export CRAYPE_LINK_TYPE=dynamic

echo "============================="
echo "  BUILDING AND COMPILATION"
echo "============================="

cd ../
make clean
make all -j

echo "================================"
echo "       RUNNING BENCHMARKS"
echo "================================"

number_of_nodes=(64)
ranks_per_node=2
N=(34)
n_repetitions=10
export OMP_NUM_THREADS=18

path_to_executable="./examples_giulia/dj.exe"
path_to_file="./benchmarks/weak-dj-bench-part2.txt"

# iterate over the values of nodes
for i in "${!number_of_nodes[@]}"
do
    nodes=${number_of_nodes[$i]}
    arg=${N[$i]}

    echo "CONFIGURATION: N = ${arg}, nodes = ${nodes}"
    echo "CONFIGURATION: N = ${arg}, nodes = ${nodes}" >> ${path_to_file}

    output=$(srun -u -N $nodes --ntasks-per-node=$ranks_per_node ${path_to_executable} -N ${arg} -r ${n_repetitions})
    echo "$output" >> ${path_to_file}
    echo "$output"
    echo "--------------------------------" >> ${path_to_file}

    echo ""
    echo "================================"
    echo ""
done
