#ifndef PARTIES_LIST_H
#define PARTIES_LIST_H

#include <vector>
#include <memory>
#include <string>
#include "Party.h"

class PartiesList {
public:
    explicit PartiesList(const std::string& filename);
    std::vector<Party*> getParties() const;
private:
    std::vector<std::unique_ptr<Party>> parties;
    void InitParties(const std::string& filename);
};

#endif
