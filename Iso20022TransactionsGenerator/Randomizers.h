#pragma once
#include <cstdint>
#include <random>

class Randomizers {
private:
    std::mt19937_64 rng_;
    std::normal_distribution<double> normal_dist_;
    double outlier_prob_;
    double outlier_mult_mean_;
public:
    Randomizers(double mean, double sd, double outlier_prob, double outlier_mult_mean);
	void setSeed(uint64_t seed);
    size_t RandomUniformInt(size_t start, size_t end);
    double NormalDistWithNoize();
};
