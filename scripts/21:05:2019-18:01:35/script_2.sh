#!/bin/bash -l
#SBATCH --time=03:00:00
#SBATCH --nodes=2
#SBATCH --constraint=mc
#SBATCH --partition=normal
#SBATCH --output="script_2.out"
#set -x

echo "================================"
echo "       RUNNING BENCHMARKS"
echo "================================"

directory=21:05:2019-18:01:35

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

N_dj=(16 19 22 24 26 27 28 29 30 31 33 )
nrep=10

N_rqc=(4 5 6 )
depth_rqc=(20 40 60 )
nrep_rqc=10_RQC


n_tasks_per_node=2
n_threads_per_task=18

nodes=2
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

