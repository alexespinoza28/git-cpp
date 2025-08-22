#include "Blob.hpp"
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>   // link with -lcrypto

static std::vector<unsigned char> readFileBytes(const std::filesystem::path& p) {
    if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p)) {
        throw std::runtime_error("Blob: file not found: " + p.string());
    }
    std::ifstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error("Blob: failed to open: " + p.string());
    std::vector<unsigned char> buf(static_cast<size_t>(std::filesystem::file_size(p)));
    if (!f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()))) {
        throw std::runtime_error("Blob: read failed: " + p.string());
    }
    return buf;
}

static std::string toHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return oss.str();
}

static std::string sha1_hex(const std::vector<unsigned char>& data) {
    // For SHA-256 instead, use EVP_sha256() and 32-byte buffer.
    const EVP_MD* md = EVP_sha1();
    unsigned char out[EVP_MAX_MD_SIZE];
    unsigned int outlen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("Blob: EVP_MD_CTX_new failed");

    if (EVP_DigestInit_ex(ctx, md, nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data.data(), data.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, out, &outlen) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Blob: EVP sha1 failed");
    }
    EVP_MD_CTX_free(ctx);
    return toHex(out, outlen); // 20 bytes -> 40 hex chars
}

Blob::Blob(const std::string& fileName, const std::string& blobName)
    : fileName(fileName), blobName(blobName)
{
    storedFile = std::filesystem::current_path() / fileName;

    // Read contents and hash them
    fileContents = readFileBytes(storedFile);
    fileHash = sha1_hex(fileContents);
}