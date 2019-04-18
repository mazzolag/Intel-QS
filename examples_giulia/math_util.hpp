#include <iostream>
#include <cmath>
#include <vector>
#include <numeric>
#include <chrono>
#include <random>
//#include "cxxopts.hpp"
#include <algorithm>
#include <tuple>
#include <omp.h>
#include <stdio.h>
#include <cstdlib>

typedef long long qInt;

bool isPrime(qInt n) { // TODO: Change this function, need something with complexity O(log(N))
    if (n == 1) {
        return false;
    }

    auto max_divisor = (qInt) std::ceil(std::sqrt(n));
    for (qInt i = 2; i <= max_divisor; ++i) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

// define gcd
qInt gcd(qInt x, qInt y){
    while (y != 0){
        qInt r = x % y;
        x = y;
        y = r;
    }
    return x;
}

// Cancel the fractions, for rational numbers z = x/y
void cancelFraction(qInt &x, qInt &y){
    qInt temp = gcd(x, y);
    x /= temp;
    y /= temp;
}

// define the function for modular exponentiation
qInt modExp(qInt a, qInt N, qInt x){
    // we use the identity: a^(i+1) % N = a*(a^i % N) % N to avoid overflow
    qInt r = 1;
    for (qInt i = 0; i < x; ++i) {
        r = r * a % N;
    }
    return r;
}

template<typename T>
T ceil_log2(T n) {
    return std::ceil(std::log2<T>(n));
}

template<typename T>
std::pair<T, T> computeStatistics(std::vector<T> vec) {
    T sum = 0, mean = 0, standardDeviation = 0;
    for (int i = 0; i < vec.size(); ++i) {
        sum += vec[i];
    }
    mean = 1.0 * sum / vec.size();

    for (int i = 0; i < vec.size(); ++i) {
        standardDeviation += std::pow(vec[i] - mean, 2);
    }
    standardDeviation = std::sqrt(1.0 * standardDeviation / vec.size());
    return {mean, standardDeviation};
}

const double pi = 3.14159265358979323846;

struct RandomGen {
    std::mt19937 mt;
    std::random_device rd;
    unsigned long int seed=0;

    RandomGen(){
        setSeed(rd());
    }

    qInt sampleUniformly(qInt low = 0, qInt high = 1){
        std::uniform_int_distribution<qInt> distribution(low, high);
        return distribution(mt);
    }

    qInt sampleDiscrete(double p0, double p1){
        std::discrete_distribution<qInt> distribution({p0, p1});
        return distribution(mt);
    }
    void setSeed (unsigned long int sd){
        seed = sd;
        mt.seed(seed);
    }
};
