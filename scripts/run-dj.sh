#!/bin/bash -l

# this part will be ignored if:
# - nodes are preallocated with salloc
# - and the file is executed as just run.sh (not sbatch run.sh)
#SBATCH --nodes=16
#SBATCH --ntasks-per-node=2
#SBATCH --constraint=mc
#SBATCH --partiition=normal
#SBATCH --time=400

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

number_of_nodes=(1 2 4 8 16)
ranks_per_node=2

n_repetitions=10
N=(16 19 22 24 26 27 28 29 30 31 33)

export OMP_NUM_THREADS=18

path_to_executable="./examples_giulia/dj.exe"
path_to_file="./benchmarks/dj-benchmark.txt"

# iterate over the values of argument N
for arg in "${N[@]}"
do
    # iterate over the values of threads
    for nodes in "${number_of_nodes[@]}"
    do

        echo "CONFIGURATION: N = ${arg}, nodes = ${nodes}"
        echo "CONFIGURATION: N = ${arg}, nodes = ${nodes}" >> ${path_to_file}

        output=$(srun -u -N $nodes --ntasks-per-node=$ranks_per_node ${path_to_executable} -N ${arg} -r ${n_repetitions})
        echo "$output" >> ${path_to_file}
        echo "$output"
        echo "--------------------------------" >> ${path_to_file}
    done
    echo ""
    echo "================================"
    echo ""
done
