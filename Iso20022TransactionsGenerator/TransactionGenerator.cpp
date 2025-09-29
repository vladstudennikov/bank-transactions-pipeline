#include "TransactionGenerator.h"

#include <random>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <iostream>

template <typename T>
inline T clamp(const T& val, const T& lo, const T& hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

TransactionGenerator::TransactionGenerator(double mean, double sd, double outlier_prob, double outlier_mult_mean)
    : rng_(std::random_device{}()),
    normal_dist_(mean, sd),
    outlier_prob_(outlier_prob),
    outlier_mult_mean_(outlier_mult_mean),
    counter_(0)
{
    InitParties("Data/parties.txt");
}

void TransactionGenerator::SetSeed(uint64_t seed) {
    rng_.seed(seed);
}

void TransactionGenerator::InitParties(const std::string& filename) {
    parties_.clear();

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string name, iban;

        if (std::getline(ss, name, ',') && std::getline(ss, iban)) {
            parties_.push_back({ name, iban });
        }
    }
    file.close();
}

size_t TransactionGenerator::RandIndex() {
    std::uniform_int_distribution<size_t> dist(0, parties_.size() - 1);
    return dist(rng_);
}

double TransactionGenerator::SampleAmount() {
    double base = normal_dist_(rng_);
    if (base < 1.0) base = 1.0;

    std::uniform_real_distribution<double> prob(0.0, 1.0);
    if (prob(rng_) < outlier_prob_) {
        std::lognormal_distribution<double> logn(std::log(outlier_mult_mean_), 1.0);
        double mult = logn(rng_);
        double big = base * mult;
        return clamp(big, 1.0, 1e9);
    }
    return base;
}

void TransactionGenerator::AppendAmountTwoDecimals(std::string& dest, double amount) {
    long long cents = static_cast<long long>(std::llround(amount * 100.0));
    long long whole = cents / 100;
    int frac = static_cast<int>(std::llabs(cents % 100));

    dest.append(std::to_string(whole));
    dest.push_back('.');
    if (frac < 10) dest.push_back('0');
    dest.append(std::to_string(frac));
}

std::string TransactionGenerator::NowUtcIso() {
    using namespace std::chrono;
    auto now = system_clock::now();
    time_t t = system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02dZ",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    return std::string(buf);
}

std::string TransactionGenerator::GeneratePain001Fast(const std::string& msgId,
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
    const Party& debtor = parties_[RandIndex()];
    size_t creditorIdx;
    do {
        creditorIdx = RandIndex();
    } while (parties_[creditorIdx].iban == debtor.iban || creditorIdx == (&debtor - &parties_[0]));
    const Party& creditor = parties_[creditorIdx];

    double amount = SampleAmount();

    uint64_t id = ++counter_;
    std::string msgId = "MSG-" + std::to_string(id);
    std::string endToEnd = "E2E-" + std::to_string(id);
    std::string timestamp = NowUtcIso();

    return GeneratePain001Fast(msgId, timestamp,
        debtor.name, debtor.iban,
        creditor.name, creditor.iban,
        endToEnd, amount, "EUR");
}

std::vector<std::string> TransactionGenerator::GenerateBatch(size_t n) {
    std::vector<std::string> out;
    out.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        out.push_back(GenerateRandomTransaction());
    }
    return out;
}