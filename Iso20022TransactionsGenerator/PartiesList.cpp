#include "PartiesList.h"
#include <fstream>
#include <sstream>
#include <iostream>

PartiesList::PartiesList(const std::string& filename) {
    InitParties(filename);
}

std::vector<Party*> PartiesList::getParties() const {
    std::vector<Party*> result;
    result.reserve(parties.size());
    for (auto& p : parties) {
        result.push_back(p.get());
    }
    return result;
}

void PartiesList::InitParties(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string name, iban;

        if (std::getline(ss, name, ',') && std::getline(ss, iban)) {
            parties.push_back(std::make_unique<Party>(name, iban));
        }
    }
}