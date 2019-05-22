#!/bin/bash -l
#SBATCH --time=GLOBAL_TIME
#SBATCH --nodes=GLOBAL_NODES
#SBATCH --constraint=mc
#SBATCH --partition=normal
#SBATCH --output="script_GLOBAL_NODES.out"
#set -x

echo "================================"
echo "       RUNNING BENCHMARKS"
echo "================================"

directory=GLOBAL_DIRECTORY

prefix=/scratch/snx3000/mazzolag/qsim/Intel-QS

cd $prefix/

run_algo(){
    path_to_executable=$1
    nodes=$2
    ranks_per_node=$3
    arg=$4
    n_repetitions=$5
    path_to_file=$6

    echo "CONFIGURATION: N = ${arg}, nodes = ${nodes}"
    echo "CONFIGURATION: N = ${arg}, nodes = ${nodes}" >> ${path_to_file}

    output=$(srun -u -N $nodes --ntasks-per-node=$ranks_per_node ${path_to_executable} -N ${arg} -r ${n_repetitions})
    echo "$output" >> ${path_to_file}
    echo "$output"
    echo "--------------------------------" >> ${path_to_file}
}

run_rqc(){
    path_to_executable=$1
    nodes=$2
    ranks_per_node=$3
    arg=$4
    dep=$5
    n_repetitions=$6
    path_to_file=$7

    export PREFIX=scrpt

    echo "CONFIGURATION: n x m = ${arg}x${arg}, depth = ${dep}, nodes = ${nodes}"
    echo "CONFIGURATION: n x m = ${arg}x${arg}, depth = ${dep}, nodes = ${nodes}" >> ${path_to_file}

    output=$(srun -u -N $nodes --ntasks-per-node=$ranks_per_node ${path_to_executable} -n ${arg} -m ${arg} -d ${dep} -r ${n_repetitions})
    echo "$output" >> ${path_to_file}
    echo "$output"
    echo "--------------------------------" >> ${path_to_file}

    echo "CONFIGURATION: n x m = ${arg}x$((${arg}+1)), depth = ${dep}, nodes = ${nodes}"
    echo "CONFIGURATION: n x m = ${arg}x$((${arg}+1)), depth = ${dep}, nodes = ${nodes}" >> ${path_to_file}

    output=$(srun -u -N $nodes --ntasks-per-node=$ranks_per_node ${path_to_executable} -n ${arg} -m $((${arg}+1)) -d ${dep} -r ${n_repetitions})
    echo "$output" >> ${path_to_file}
    echo "$output"
    echo "--------------------------------" >> ${path_to_file}

}

IFS=

N_dj=GLOBAL_N_DJ
nrep=GLOBAL_NREP

N_rqc=GLOBAL_N_RQC
depth_rqc=GLOBAL_DEPTH_RQC
nrep_rqc=GLOBAL_NREP_RQC


n_tasks_per_node=GLOBAL_TASKS
n_threads_per_task=GLOBAL_THREADS

nodes=GLOBAL_NODES
export OMP_NUM_THREADS=$n_threads_per_task


exec_dj="./examples_giulia/dj.exe"
results_dj="$prefix/scripts/$directory/dj_${nodes}.txt"

exec_rqc="./randomQC/rqc.exe"
results_rqc="$prefix/scripts/$directory/rqc_${nodes}.txt"


# run DJ algorithm
for arg in "${N_dj[@]}"
do
    run_algo ${exec_dj} $nodes ${n_tasks_per_node} $arg $nrep ${results_dj}
done

# run RQC algorithm
for arg in "${N_rqc[@]}"
do
    for dep in "${depth_rqc[@]}"
    do
        run_rqc ${exec_rqc} $nodes ${n_tasks_per_node} $arg $dep $nrep ${results_rqc}
    done
done

