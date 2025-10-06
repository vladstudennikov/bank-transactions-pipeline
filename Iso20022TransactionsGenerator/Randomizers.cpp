#include "Randomizers.h"
#include "clamp.cpp"

Randomizers::Randomizers(double mean, double sd, double outlier_prob, double outlier_mult_mean)
	: rng_(std::random_device{}()),
	normal_dist_(mean, sd),
	outlier_prob_(outlier_prob),
	outlier_mult_mean_(outlier_mult_mean) {}

void Randomizers::setSeed(uint64_t seed) {
	rng_.seed(seed);
}

size_t Randomizers::RandomUniformInt(size_t start, size_t end) {
    std::uniform_int_distribution<size_t> dist(start, end);
    return dist(rng_);
}

double Randomizers::NormalDistWithNoize() {
    double x = normal_dist_(rng_);
    std::bernoulli_distribution bd(outlier_prob_);
    if (bd(rng_)) {
        std::exponential_distribution<double> exp(1.0 / outlier_mult_mean_);
        x *= (1.0 + exp(rng_));
    }
    return clamp(x, 0.01, 1e9);
}