#!/bin/bash -l

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
