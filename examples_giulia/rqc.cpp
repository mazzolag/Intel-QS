//
// Created by giuli on 15/04/2019.
//

#pragma once
#include "../qureg/qureg.hpp"
#include "math_util.hpp"
#include <fstream>
#include <string>
using namespace std;

// apply the single qubit gates t, x^1/2 or y^1/2
void applyGate(QubitRegister<ComplexDP> &qr, const int qubit, const std::string &gate) {
    if (gate == "t"){
        qr.ApplyT(qubit);
    }
    else if (gate == "x_1_2"){
        qr.ApplyPauliSqrtX(qubit);
    }
    else if (gate == "y_1_2"){
        qr.ApplyPauliSqrtY(qubit);
    }
    else {
        std::cout << "Error: could not read gate from file" << std::endl;
        exit(1);
    }
}

void randomQuantumCircuit(int n, int d, ifstream &file, RandomGen &rng) {
    // set th input position indicator to the first element of file
    file.seekg(0);

    // creating registers
    QubitRegister<ComplexDP> qureg(n, "++++", 0); // initialize all qubits to an equal superposition state by the use of "++++"

    // skip the first n+1 lines
    std::string trashline;
    for (int j = 0; j < n+1; ++j) {
        std::getline(file, trashline);
    }

    // build circuit from config file
    std::string gate;
    int qubit1, qubit2;
    int step;
    file >> step;
    for (int i = 1; i < d; ++i) {
        while (step == i){
            file >> gate;
            if (gate == "cz"){
                file >> qubit1 >> qubit2;
                qureg.ApplyCPauliZ(qubit1, qubit2);
            }
            else {
                file >> qubit1;
                applyGate(qureg, qubit1, gate);
            }
            file >> step;
        }
    }

    // apply hadamard on all qubits (final step)
    for (int i = 0; i < n; ++i) {
        qureg.ApplyHadamard(i);
    }

    // measure the qubits
    int value = (int) rng.sampleUniformly(0,1);
    for (int l = 0; l < n; ++l) {
        qureg.CollapseQubit(l, value);
    }

}

int main(int argc, char **argv){
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
    int rows = 4;
    int cols = 4;
    int d = 10;
    int I = 0;
    int nrep = 1;
    while ((argc > 1) && (argv[1][0] == '-')){
        switch (argv[1][1]){
            case 'n':  // Configuration size n x m: Number n of rows (n ranging from 4 to 11)
                rows = std::atoi(argv[2]);
                break;
            case 'm':  // Configuration size n x m: Number m if columns (m = n or m = n+1)
                cols = std::atoi(argv[2]);
                break;
            case 'd':  // Depth / Number of clock cycles (d ranging from 10 to 80)
                d = std::atoi(argv[2]);
                break;
            case 'I':  // Choose a particular circuit from a set of 10 different random circuits examples (I ranging from 0 to 9)
                I = std::atoi(argv[2]);
                break;
            case 'r':  // Number of repetitions
                nrep = std::atoi(argv[2]);
                break;
            default:
                std::cout << "Wrong Argument: " << argv[1] << std::endl;
        }
        argv += 2;
        argc -= 2;
    }

    // check for correct input configurations / output error messages
    if (rows > 11 || rows < 4){
        std::cout << "Error: the number n of rows must take a value between 4 and 11" << std::endl;
        exit(1);
    }
    if ((cols - rows) > 1 || (rows - cols) > 0){
        std::cout << "Error: the number m of columns must be either m = n or m = n+1" << std::endl;
        exit(1);
    }
    if (d > 80 || d < 10){
        std::cout << "Error: the depth d must take a value between 10 and 80" << std::endl;
        exit(1);
    }
    if (I > 9 || I < 0){
        std::cout << "Error: the exampleID I must take a value between 0 and 9" << std::endl;
        exit(1);
    }

    // add prefix for path variable
    std::string prefix = "../";
    const char * pf = std::getenv("PREFIX");
    if (pf){
        std::string s(pf, 5);
    
        if (s == "scrpt") {
            prefix = "./";
        }
    }

    // read gate configurations from Boixo files (Google)
    std::string config = std::to_string(rows) + "x" + std::to_string(cols);
    std::string configfile = "inst_" + config + "_" + std::to_string(d) + "_" + std::to_string(I) + ".txt";
    ifstream inFile;
    std::string path = prefix + "examples_giulia/cz_v2/" + config + "/" + configfile;
    inFile.open(path);
    if (!inFile) {
        cerr << "Error: unable to open the configuration file";
        exit(1);   // call system to stop
    }

    int n = rows*cols; // number of qubits
    std::vector<long long> times;
    times.reserve(nrep);

    // create RNG
    RandomGen rng;

    // run the ciruit
    for (int i = 0; i < nrep; ++i) {
        MPI_Barrier(MPI_COMM_WORLD);
        auto start = std::chrono::steady_clock::now();
        randomQuantumCircuit(n, d, inFile, rng);
        MPI_Barrier(MPI_COMM_WORLD);
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

    inFile.close();
    return 0;
}
