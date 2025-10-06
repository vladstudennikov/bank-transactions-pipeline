#include "UtcIsoTimeGenerator.h"
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

std::string UtcIsoTimeGenerator::NowUtcIso() {
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