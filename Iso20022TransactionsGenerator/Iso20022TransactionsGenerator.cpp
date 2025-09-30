// Iso20022TransactionsGenerator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "TransactionGenerator.h"

int main()
{
    TransactionGenerator gen("parties.txt");

    auto batch = gen.GenerateBatch(10);
    for (size_t i = 0; i < batch.size(); ++i) {
        std::cout << "---- Transaction " << (i + 1) << " ----\n";
        std::cout << batch[i] << "\n";
    }

    return 0;
}