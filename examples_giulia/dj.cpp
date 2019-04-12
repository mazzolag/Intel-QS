//------------------------------------------------------------------------------
// Created by giuli on 11/04/2019.
//------------------------------------------------------------------------------
#pragma once
#include "../qureg/qureg.hpp"
#include "math_util.hpp"


void dj(int n, RandomGen &rng) {

    // Create a type of function (either constant or balanced) at random (oracle)
    int oracleType = (int) rng.sampleUniformly(0,1);
    int oracleValue = (int) rng.sampleUniformly(0,1);
    std::cout << "oracleType = " << oracleType << ", oracleValue = " << oracleValue << std::endl;
    std::string type = oracleType == 0 ? "a constant valued function" : "a balanced function";
    std::cout << "The oracle returns " << type << std::endl;

    // choose a balanced function at random and represent it by the long long value f
    qInt f = rng.sampleUniformly(1, (1ll << n) - 1);
    // std::cout << "f = " << f << std::endl;

    // Construct the circuit
    // creating registers
    QubitRegister<ComplexDP> qureg(n+1, "++++", 0); // initialize all qubits to an equal superposition state by the use of "++++"
    qureg.ApplyPauliZ(n); // necessary in order to apply the phase oracle

    //implement the bit oracle
    if (oracleType == 0){ // If the oracleType is "0" (constant function), the oracle returns oracleValue for all input
        if (oracleValue == 1){
            qureg.ApplyPauliX(n);
        }
    }
    else {
        for (int i = 0; i < n ; ++i) { // this represents a way to define balanced functions which are represented by f
            if (f & (1ll << i)){
                qureg.ApplyCPauliX(i,n);
            }

        }

    }

    // Apply again hadamard on all qubits
    for (int i = 0; i < n; ++i) {
        qureg.ApplyHadamard(i);
    }

    // measure the qubits
    double realprob = qureg.GetStateProb(0) + qureg.GetStateProb(1ll << n); // get the probability amplitude of state |00.....0>
    // NOTE: If the algorithm runs with real hardware backend, need to change the above line and measure all qubits separately.

    //int prob = int(realprob);


    if (realprob > 0.5){
        std::cout << "The DJ Algorithm finds the function f to be constant with value " << oracleValue << std::endl;
    }
    else {
        std::cout << "The DJ Algorithm finds the function f to be balanced." << std::endl;
    }

}  


// Start of the main program (C++ language).
int main(int argc, char **argv)
{
    // Create the MPI environment, passing the same argument to all the ranks.
    openqu::mpi::Environment env(argc, argv);
    // qHiPSTER is structured so that only even number of ranks are used to store
    // and manipulate the quantum state. In case the number of ranks is not supported,
    // try to decrease it by 1 until it is.
    if (env.is_usefull_rank() == false) return 0;
    // qHiPSTER has functions that simplify some MPI instructions. However, it is important
    // to keep trace of the current rank.
    int myid = env.rank();


    // command line arguments
    int n = 13;
    int nrep = 1;
    while ((argc > 1) && (argv[1][0] == '-')){
        switch (argv[1][1]){
            case 'N':
                n = std::atoi(argv[2]);
                break;
            case 'r':
                nrep = std::atoi(argv[2]);
                break;
            default:
                std::cout << "Wrong Argument: " << argv[1] << std::endl;
        }
        argv += 2;
        argc -= 2;
    }

    // create RNG
    RandomGen rng;

    std::vector<long long> times;
    times.reserve(nrep);

    std::vector<qInt> factors;

    //rng.setSeed(0);
    for (int i = 0; i < nrep; ++i) {	
	    //rng.setSeed(0);
	    // syncQuESTEnv(env);
        auto start = std::chrono::steady_clock::now();
        dj(n, rng);
        // syncQuESTEnv(env);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        times.push_back(duration);
    }

    if (myid == 0) {
        // output the duration vector over repetitions
        std::cout << "Duration [ms] = ";
        for (int i = 0; i < nrep; ++i) {
            std::cout << times[i];
            if (i < nrep - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;

        auto statistics = computeStatistics(times);
        std::cout << "Avg time [ms] = " << statistics.first << ", Std deviation = " << statistics.second << std::endl;
    }

    return 0;
}
