#include "StreamingTransactionsGenerator.h"
#include <chrono>
#include <thread>

StreamingTransactionsGenerator& StreamingTransactionsGenerator::Instance() {
	static StreamingTransactionsGenerator instance;
	return instance;
}

void StreamingTransactionsGenerator::Init(std::unique_ptr<TransactionGenerator> generator) {
	gen = std::move(generator);
}

void StreamingTransactionsGenerator::GenerateTransactionsPeriodically(
    size_t batchSize,
    int periodMs,
    std::function<void(const std::vector<std::string>&)> callback
) {
    if (!gen) return;

    while (true) {
        auto batch = gen->GenerateBatch(batchSize);
        callback(batch);
        std::this_thread::sleep_for(std::chrono::milliseconds(periodMs));
    }
}