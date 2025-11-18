#include "model/Task.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

std::string nowIso8601UTC() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::string trim(const std::string& s) {
    auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c){ return std::isspace(c); });
    auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c){ return std::isspace(c); }).base();
    if (wsback <= wsfront) return {};
    return std::string(wsfront, wsback);
}
