#!/bin/bash -l

# this part will be ignored if:
# - nodes are preallocated with salloc
# - and the file is executed as just run.sh (not sbatch run.sh)
#SBATCH --nodes=256
#SBATCH --constraint=mc
#SBATCH --partition=normal
#SBATCH --time=180

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

number_of_nodes=(256 128)
ranks_per_node=2

n_repetitions=4
N=(5)
depth=(40)

export OMP_NUM_THREADS=18
export PREFIX=scrpt

path_to_executable="./examples_giulia/rqc.exe"
path_to_file="./benchmarks/rqc-plus-2.txt"

# iterate over the values of argument N
for arg in "${N[@]}"
do

    # iterate over the values of depth
    for dep in "${depth[@]}"
    do

        # iterate over the values of nodes (with m = n)
        for nodes in "${number_of_nodes[@]}"
        do

           echo "CONFIGURATION: n x m = ${arg}x${arg}, depth = ${dep}, nodes = ${nodes}"
           echo "CONFIGURATION: n x m = ${arg}x${arg}, depth = ${dep}, nodes = ${nodes}" >> ${path_to_file}

            output=$(srun -u -N $nodes --ntasks-per-node=$ranks_per_node ${path_to_executable} -n ${arg} -m ${arg} -d ${dep} -r ${n_repetitions})
            echo "$output" >> ${path_to_file}
            echo "$output"
            echo "--------------------------------" >> ${path_to_file}
        done
        echo ""
        echo "================================"
        echo ""

        # iterate over the values of nodes (with m = n+1)
        for nodes in "${number_of_nodes[@]}"
        do

            echo "CONFIGURATION: n x m = ${arg}x$((${arg}+1)), depth = ${dep}, nodes = ${nodes}"
            echo "CONFIGURATION: n x m = ${arg}x$((${arg}+1)), depth = ${dep}, nodes = ${nodes}" >> ${path_to_file}

            output=$(srun -u -N $nodes --ntasks-per-node=$ranks_per_node ${path_to_executable} -n ${arg} -m $((${arg}+1)) -d ${dep} -r ${n_repetitions})
            echo "$output" >> ${path_to_file}
            echo "$output"
            echo "--------------------------------" >> ${path_to_file}
        done
        echo ""
        echo "================================"
        echo ""

    done
done
