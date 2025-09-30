#include "Party.h"

Party::Party(const std::string& name, const std::string& iban)
    : name(name),
    iban(iban) {}

const std::string& Party::getName() const noexcept {
    return name;
}

const std::string& Party::getIban() const noexcept {
    return iban;
}

void Party::setName(const std::string& newName) { 
    name = newName; 
}

void Party::setName(std::string&& newName) { 
    name = std::move(newName); 
}

void Party::setIban(const std::string& newIban) {
    iban = newIban;
}

void Party::setIban(std::string&& newIban) {
    iban = std::move(newIban);
}