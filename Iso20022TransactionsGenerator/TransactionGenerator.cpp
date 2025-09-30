#include "TransactionGenerator.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <random>

static double clamp(double x, double lo, double hi) {
    return (x < lo) ? lo : (x > hi ? hi : x);
}

TransactionGenerator::TransactionGenerator(const std::string& partiesFile,
    double mean, double sd, double outlier_prob, double outlier_mult_mean)
    : rng_(std::random_device{}()),
    normal_dist_(mean, sd),
    outlier_prob_(outlier_prob),
    outlier_mult_mean_(outlier_mult_mean),
    counter_(0)
{
    this->partiesList = std::make_unique<PartiesList>(partiesFile);
}

void TransactionGenerator::SetSeed(uint64_t seed) {
    rng_.seed(seed);
}

size_t TransactionGenerator::RandIndex() {
    auto parties = partiesList->getParties();
    std::uniform_int_distribution<size_t> dist(0, parties.size() - 1);
    return dist(rng_);
}

double TransactionGenerator::SampleAmount() {
    double x = normal_dist_(rng_);
    std::bernoulli_distribution bd(outlier_prob_);
    if (bd(rng_)) {
        std::exponential_distribution<double> exp(1.0 / outlier_mult_mean_);
        x *= (1.0 + exp(rng_));
    }
    return clamp(x, 0.01, 1e9);
}

void TransactionGenerator::AppendAmountTwoDecimals(std::string& dest, double amount) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.2f", amount);
    dest += buf;
}

std::string TransactionGenerator::NowUtcIso() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
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
    auto parties = partiesList->getParties();

    const Party* debtor = parties[RandIndex()];
    const Party* creditor = nullptr;

    do {
        creditor = parties[RandIndex()];
    } while (creditor->getIban() == debtor->getIban());

    double amount = SampleAmount();

    uint64_t id = ++counter_;
    std::string msgId = "MSG-" + std::to_string(id);
    std::string endToEnd = "E2E-" + std::to_string(id);
    std::string timestamp = NowUtcIso();

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