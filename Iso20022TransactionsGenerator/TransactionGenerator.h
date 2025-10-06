#ifndef TRANSACTION_GENERATOR_H
#define TRANSACTION_GENERATOR_H

#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <memory>
#include "Party.h"
#include "PartiesList.h"

class TransactionGenerator {
public:
    TransactionGenerator(const std::string& partiesFile);
    std::string GenerateRandomTransaction();
    std::vector<std::string> GenerateBatch(size_t n);

private:
    static void AppendAmountTwoDecimals(std::string& dest, double amount);
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
    std::unique_ptr<PartiesList> partiesList;
    std::atomic<uint64_t> counter_;
};

#endif // TRANSACTION_GENERATOR_H