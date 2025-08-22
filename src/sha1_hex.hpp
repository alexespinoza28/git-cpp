#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <CommonCrypto/CommonDigest.h>

inline std::string sha1_hex(const std::vector<unsigned char>& v) {
    unsigned char out[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(v.data(), static_cast<CC_LONG>(v.size()), out);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < sizeof(out); ++i)
        oss << std::setw(2) << static_cast<unsigned>(out[i]);
    return oss.str();
}

inline std::string sha1_hex(const std::string& s) {
    const auto* p = reinterpret_cast<const unsigned char*>(s.data());
    unsigned char out[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(p, static_cast<CC_LONG>(s.size()), out);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < sizeof(out); ++i)
        oss << std::setw(2) << static_cast<unsigned>(out[i]);
    return oss.str();
}
