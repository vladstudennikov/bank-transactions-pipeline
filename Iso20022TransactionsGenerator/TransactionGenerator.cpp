#include "TransactionGenerator.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <random>
#include "clamp.cpp"
#include "Randomizers.h"
#include "UtcIsoTimeGenerator.h"

TransactionGenerator::TransactionGenerator(const std::string& partiesFile)
    : counter_(0) 
{
    this->partiesList = std::make_unique<PartiesList>(partiesFile);
}

void TransactionGenerator::AppendAmountTwoDecimals(std::string& dest, double amount) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.2f", amount);
    dest += buf;
}

std::string TransactionGenerator::GeneratePain001Fast(
    const std::string& msgId,
    const std::string& timestamp,
    const std::string& debtorName,
    const std::string& debtorIBAN,
    const std::string& creditorName,
    const std::string& creditorIBAN,
    const std::string& endToEndId,
    double amount,
    const std::string& currency)
{
    std::string xml;
    xml.reserve(800 + debtorName.size() + creditorName.size());

    xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml += "<Document xmlns=\"urn:iso:std:iso:20022:tech:xsd:pain.001.001.03\">\n";
    xml += "  <CstmrCdtTrfInitn>\n";
    xml += "    <GrpHdr>\n";
    xml += "      <MsgId>";
    xml += msgId;
    xml += "</MsgId>\n";
    xml += "      <CreDtTm>";
    xml += timestamp;
    xml += "</CreDtTm>\n";
    xml += "      <NbOfTxs>1</NbOfTxs>\n";
    xml += "      <CtrlSum>";
    AppendAmountTwoDecimals(xml, amount);
    xml += "</CtrlSum>\n";
    xml += "      <InitgPty><Nm>";
    xml += debtorName;
    xml += "</Nm></InitgPty>\n";
    xml += "    </GrpHdr>\n";

    xml += "    <PmtInf>\n";
    xml += "      <PmtInfId>PmtInf-";
    xml += std::to_string(counter_.load());
    xml += "</PmtInfId>\n";
    xml += "      <PmtMtd>TRF</PmtMtd>\n";
    xml += "      <NbOfTxs>1</NbOfTxs>\n";
    xml += "      <CtrlSum>";
    AppendAmountTwoDecimals(xml, amount);
    xml += "</CtrlSum>\n";

    xml += "      <Dbtr><Nm>";
    xml += debtorName;
    xml += "</Nm></Dbtr>\n";

    xml += "      <DbtrAcct><Id><IBAN>";
    xml += debtorIBAN;
    xml += "</IBAN></Id></DbtrAcct>\n";

    xml += "      <CdtTrfTxInf>\n";
    xml += "        <PmtId><EndToEndId>";
    xml += endToEndId;
    xml += "</EndToEndId></PmtId>\n";
    xml += "        <Amt><InstdAmt Ccy=\"";
    xml += currency;
    xml += "\">";
    AppendAmountTwoDecimals(xml, amount);
    xml += "</InstdAmt></Amt>\n";
    xml += "        <Cdtr><Nm>";
    xml += creditorName;
    xml += "</Nm></Cdtr>\n";
    xml += "        <CdtrAcct><Id><IBAN>";
    xml += creditorIBAN;
    xml += "</IBAN></Id></CdtrAcct>\n";
    xml += "      </CdtTrfTxInf>\n";

    xml += "    </PmtInf>\n";
    xml += "  </CstmrCdtTrfInitn>\n";
    xml += "</Document>\n";

    return xml;
}

std::string TransactionGenerator::GenerateRandomTransaction() {
    Randomizers r(1000.0, 300.0, 0.01, 50.0);
    auto parties = partiesList->getParties();

    const Party* debtor = parties[r.RandomUniformInt(0, partiesList->getParties().size() - 1)];
    const Party* creditor = nullptr;

    do {
        creditor = parties[r.RandomUniformInt(0, partiesList->getParties().size() - 1)];
    } while (creditor->getIban() == debtor->getIban());

    double amount = r.NormalDistWithNoize();

    uint64_t id = ++counter_;
    std::string msgId = "MSG-" + std::to_string(id);
    std::string endToEnd = "E2E-" + std::to_string(id);
    std::string timestamp = UtcIsoTimeGenerator::NowUtcIso();

    return GeneratePain001Fast(msgId, timestamp,
        debtor->getName(), debtor->getIban(),
        creditor->getName(), creditor->getIban(),
        endToEnd, amount, "EUR");
}

std::vector<std::string> TransactionGenerator::GenerateBatch(size_t n) {
    std::vector<std::string> batch;
    batch.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        batch.push_back(GenerateRandomTransaction());
    }
    return batch;
}