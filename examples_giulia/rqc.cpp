//
// Created by giuli on 02/04/2019.
//

#include "../examples_giulia/math_util.hpp"
#include "../examples_giulia/cxxopts.hpp"
#include <fstream>
#include <string>
#include <cstdlib>
using namespace std;

// apply the single qubit gates t, x^1/2 or y^1/2
void applyGate(Qureg &qr, const int qubit, const std::string &gate){
    if (gate == "t"){
        tGate(qr, qubit);
    }
    else if (gate == "x_1_2"){
        rotateX(qr, qubit, pi/2);
    }
    else if (gate == "y_1_2"){
        rotateY(qr, qubit, pi/2);
    }
    else {
        std::cout << "Error: could not read gate from file" << std::endl;
        exit(1);
    }
}

void randomQuantumCircuit(QuESTEnv &env, int n, int d, ifstream &file){
    // set th input position indicator to the first element of file
    file.seekg(0);

    Qureg qureg = createQureg(n, env); // create quantum register
    initPlusState(qureg); // apply hadamard on all qubits (first step)

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
                controlledPhaseFlip(qureg, qubit1, qubit2);
            }
            else {
                file >> qubit1;
                applyGate(qureg, qubit1, gate);
            }
            file >> step;
        }
    }

    // apply hadamard on all qubits (final step)
    for (int k = 0; k < n; ++k) {
        hadamard(qureg,k);
    }

    // measure the qubits
    for (int l = 0; l < n; ++l) {
        measure(qureg, l);
    }

    destroyQureg(qureg, env);
}

int main(int narg, char *varg[]){
    //load quest
    QuESTEnv env = createQuESTEnv();
    // command lines for input variables
    cxxopts::Options desc(varg[0], "Allowed Options");
    desc.add_options()
            ("help", "produce help message")
            ("n,nrow", "Configuration size n x m: Number n of rows (n ranging from 4 to 11)", cxxopts::value<int>()->default_value("4"))
            ("m,mcolumn", "Configuration size n x m: Number m of columns (m = n or m = n+1)", cxxopts::value<int>()->default_value("4"))
            ("d,depth", "Depth / Number of clock cycles (d ranging from 10 to 80)", cxxopts::value<int>()->default_value("10"))
            ("I,exampleId", "Choose a particular circuit from a set of 10 different random circuits examples (I ranging from 0 to 9", cxxopts::value<int>()->default_value("0"))
            ("r,repetitions", "Number of repetitions", cxxopts::value<int>()->default_value("1"));

    auto vm = desc.parse(narg, varg);

    if (vm.count("help")) {
        std::cout << desc.help() << std::endl;
        return 1;
    }

    const int rows = vm["n"].as<int>();
    const int cols = vm["m"].as<int>();
    const int d = vm["d"].as<int>();
    const int I = vm["I"].as<int>();
    const int nrep = vm["r"].as<int>();

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
    std::string prefix = "../../";
    const char * pf = std::getenv("PREFIX");
    std::string s(pf, 5);
    // std::cout << "path s = " << s << std::endl;
    if (s == "local") {
        prefix += "../";
    }

    if (s == "scrpt") {
	prefix = "../";
    } 

    // read gate configurations from Boixo files (Google)
    std::string config = std::to_string(rows) + "x" + std::to_string(cols);
    std::string configfile = "inst_" + config + "_" + std::to_string(d) + "_" + std::to_string(I) + ".txt";
    ifstream inFile;
    std::string path = prefix + "randomQC/cz_v2/" + config + "/" + configfile;
    inFile.open(path);
    if (!inFile) {
        cerr << "Error: unable to open the configuration file";
        exit(1);   // call system to stop
    }

    int n = rows*cols; // number of qubits
    std::vector<long long> times;
    times.reserve(nrep);

    // run the ciruit
    for (int i = 0; i < nrep; ++i) {
        syncQuESTEnv(env);
        auto start = std::chrono::steady_clock::now();
        randomQuantumCircuit(env, n, d, inFile);
        syncQuESTEnv(env);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        times.push_back(duration);
    }

    if (env.rank == 0) {
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
    destroyQuESTEnv(env);
    return 0;
}
