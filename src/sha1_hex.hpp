#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

inline std::string bytes_to_hex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return oss.str();
}

#if defined(__APPLE__)
// -------- macOS / iOS: CommonCrypto (no extra libs) --------
#include <CommonCrypto/CommonDigest.h>

inline std::string sha1_hex(const std::vector<unsigned char>& data) {
    unsigned char out[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(data.data(), static_cast<CC_LONG>(data.size()), out);
    return bytes_to_hex(out, sizeof(out));
}
inline std::string sha1_hex(const std::string& s) {
    const auto* p = reinterpret_cast<const unsigned char*>(s.data());
    unsigned char out[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(p, static_cast<CC_LONG>(s.size()), out);
    return bytes_to_hex(out, sizeof(out));
}

#elif defined(_WIN32)
// -------- Windows: CNG / BCrypt (ships with Windows) --------
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

inline std::string sha1_hex(const std::vector<unsigned char>& data) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS st;

    st = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA1_ALGORITHM, nullptr, 0);
    if (st < 0) throw std::runtime_error("BCryptOpenAlgorithmProvider failed");

    DWORD hashObjectLen = 0, cbData = 0;
    st = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjectLen, sizeof(hashObjectLen), &cbData, 0);
    if (st < 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCryptGetProperty OBJECT_LENGTH failed"); }

    std::vector<UCHAR> hashObject(hashObjectLen);
    const DWORD digestLen = 20; // SHA-1
    UCHAR digest[digestLen];

    st = BCryptCreateHash(hAlg, &hHash, hashObject.data(), hashObjectLen, nullptr, 0, 0);
    if (st < 0) { BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCryptCreateHash failed"); }

    st = BCryptHashData(hHash, const_cast<PUCHAR>(reinterpret_cast<const UCHAR*>(data.data())),
                        static_cast<ULONG>(data.size()), 0);
    if (st < 0) { BCryptDestroyHash(hHash); BCryptCloseAlgorithmProvider(hAlg, 0); throw std::runtime_error("BCryptHashData failed"); }

    st = BCryptFinishHash(hHash, digest, digestLen, 0);
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (st < 0) throw std::runtime_error("BCryptFinishHash failed");

    return bytes_to_hex(digest, digestLen);
}

inline std::string sha1_hex(const std::string& s) {
    const auto* p = reinterpret_cast<const unsigned char*>(s.data());
    std::vector<unsigned char> v(p, p + s.size());
    return sha1_hex(v);
}

#else
// -------- Other (e.g., Linux): OpenSSL fallback --------
#include <openssl/evp.h>

inline std::string sha1_hex(const std::vector<unsigned char>& data) {
    unsigned char out[EVP_MAX_MD_SIZE];
    unsigned int outlen = 0;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_MD_CTX_new failed");

    if (EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data.data(), data.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, out, &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP sha1 failed");
    }
    EVP_MD_CTX_free(ctx);
    return bytes_to_hex(out, outlen);
}

inline std::string sha1_hex(const std::string& s) {
    const auto* p = reinterpret_cast<const unsigned char*>(s.data());
    std::vector<unsigned char> v(p, p + s.size());
    return sha1_hex(v);
}
#endif
