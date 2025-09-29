#ifndef TRANSACTION_GENERATOR_H
#define TRANSACTION_GENERATOR_H

#include <string>
#include <vector>
#include <atomic>
#include <random>

struct Party {
    std::string name;
    std::string iban;
};

class TransactionGenerator {
public:
    TransactionGenerator(double mean = 1000.0, double sd = 300.0,
        double outlier_prob = 0.01, double outlier_mult_mean = 50.0);

    std::string GenerateRandomTransaction();

    std::vector<std::string> GenerateBatch(size_t n);

    void SetSeed(uint64_t seed);

private:
    void InitParties(const std::string& filename);
    size_t RandIndex();
    double SampleAmount();
    static void AppendAmountTwoDecimals(std::string& dest, double amount);
    static std::string NowUtcIso();
    std::string GeneratePain001Fast(const std::string& msgId,
        const std::string& timestamp,
        const std::string& debtorName,
        const std::string& debtorIBAN,
        const std::string& creditorName,
        const std::string& creditorIBAN,
        const std::string& endToEndId,
        double amount,
        const std::string& currency);

private:
    std::mt19937_64 rng_;
    std::normal_distribution<double> normal_dist_;
    double outlier_prob_;
    double outlier_mult_mean_;

    std::vector<Party> parties_;

    std::atomic<uint64_t> counter_;
};

#endif // TRANSACTION_GENERATOR_H