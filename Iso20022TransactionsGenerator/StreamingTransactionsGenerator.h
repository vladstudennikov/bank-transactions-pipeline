#pragma once
#include <memory>
#include "TransactionGenerator.h"
#include <functional>

class StreamingTransactionsGenerator {
private:
	std::unique_ptr<TransactionGenerator> gen;
    StreamingTransactionsGenerator() = default;

    StreamingTransactionsGenerator(const StreamingTransactionsGenerator&) = delete;
    StreamingTransactionsGenerator& operator=(const StreamingTransactionsGenerator&) = delete;

public:
    static StreamingTransactionsGenerator& Instance();
    void Init(std::unique_ptr<TransactionGenerator> generator);

    void GenerateTransactionsPeriodically(
        size_t batchSize,
        int periodMs,
        std::function<void(const std::vector<std::string>&)> callback
    );
};