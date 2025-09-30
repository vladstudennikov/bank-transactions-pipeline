#pragma once
#include <string>

#include <string>
#include <memory>

class Party {
public:
    Party(const std::string& name, const std::string& iban);

    const std::string& getName() const noexcept;
    const std::string& getIban() const noexcept;

    void setName(const std::string& newName);
    void setName(std::string&& newName);

    void setIban(const std::string& newIban);
    void setIban(std::string&& newIban);

private:
    std::string name;
    std::string iban;
};