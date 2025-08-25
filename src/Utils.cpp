#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <CommonCrypto/CommonDigest.h>

namespace gitcpp {

// sha1

std::string sha1(const std::vector<unsigned char>& v) {
    unsigned char out[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(v.data(), static_cast<CC_LONG>(v.size()), out);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < sizeof(out); ++i) {
        oss << std::setw(2) << static_cast<unsigned>(out[i]);
    }
    return oss.str();
}

std::string sha1(const std::string& s) {
    const auto* p = reinterpret_cast<const unsigned char*>(s.data());
    unsigned char out[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1(p, static_cast<CC_LONG>(s.size()), out);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < sizeof(out); ++i) {
        oss << std::setw(2) << static_cast<unsigned>(out[i]);
    }
    return oss.str();
}

// --- File I/O ---

bool restrictedDelete(const std::filesystem::path& file) {
    if (!std::filesystem::exists(file.parent_path() / ".gitcpp")) {
        return false;
    }
    if (std::filesystem::exists(file) && !std::filesystem::is_directory(file)) {
        return std::filesystem::remove(file);
    }
    return false;
}

std::vector<unsigned char> readContents(const std::filesystem::path& file) {
    if (!std::filesystem::is_regular_file(file)) {
        throw error("Not a regular file: " + file.string());
    }
    std::ifstream f(file, std::ios::binary);
    if (!f) {
        throw error("Could not open file: " + file.string());
    }
    auto size = std::filesystem::file_size(file);
    std::vector<unsigned char> buffer(size);
    if (!f.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw error("Could not read file: " + file.string());
    }
    return buffer;
}

std::string readContentsAsString(const std::filesystem::path& file) {
    auto bytes = readContents(file);
    return std::string(bytes.begin(), bytes.end());
}

void writeContents_impl_(const std::filesystem::path& file,
                         const std::vector<std::vector<unsigned char>>& chunks) {
    std::ofstream ofs(file, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        throw error("Could not open for writing: " + file.string());
    }
    for (const auto& chunk : chunks) {
        ofs.write(reinterpret_cast<const char*>(chunk.data()), chunk.size());
        if (!ofs) {
            throw error("Error while writing to: " + file.string());
        }
    }
}

std::vector<std::string> plainFilenamesIn(const std::filesystem::path& dir) {
    std::vector<std::string> filenames;
    if (std::filesystem::is_directory(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                filenames.push_back(entry.path().filename().string());
            }
        }
    }
    std::sort(filenames.begin(), filenames.end());
    return filenames;
}

// path

std::filesystem::path join(const std::string& first, std::initializer_list<std::string> others) {
    std::filesystem::path p(first);
    for (const auto& o : others) {
        p /= o;
    }
    return p;
}

std::filesystem::path join(const std::filesystem::path& first, std::initializer_list<std::string> others) {
    std::filesystem::path p(first);
    for (const auto& o : others) {
        p /= o;
    }
    return p;
}

// message

void message(const std::string& s) {
    std::cout << s << std::endl;
}

GitcppException error(const std::string& s) {
    return GitcppException(s);
}

} // namespace gitcpp
