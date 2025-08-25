#pragma once
#include "GitcppException.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <initializer_list>
#include <type_traits>

namespace gitcpp {

    /// Length of a full SHA-1 hex UID.
    inline constexpr int UID_LENGTH = 40;

    /// SHA-1 helpers (macOS: CommonCrypto).
    std::string sha1(const std::vector<unsigned char>& bytes);
    std::string sha1(const std::string& s);

    /// Variadic SHA-1 of concatenation of byte arrays and/or strings.
    /// Accepts any mix of: std::vector<unsigned char>, std::string, const char*
    template <typename... Args>
    std::string sha1_concat(const Args&... parts);

    /// Delete file if it exists and is not a directory, but only if
    /// a sibling `.gitcpp` directory exists (same policy as restrictedDelete).
    bool restrictedDelete(const std::filesystem::path& file);

    /// Read entire file as bytes (throws on errors / if not a regular file).
    std::vector<unsigned char> readContents(const std::filesystem::path& file);

    /// Read entire file as UTF-8 string.
    std::string readContentsAsString(const std::filesystem::path& file);

    /// Write concatenated contents to file (create/overwrite).
    /// Accepts any mix of std::vector<unsigned char>, std::string, const char*.
    template <typename... Args>
    void writeContents(const std::filesystem::path& file, const Args&... parts);

    /// Return sorted list of plain filenames inside a directory (or empty if not dir).
    std::vector<std::string> plainFilenamesIn(const std::filesystem::path& dir);

    /// join("a", "b", "c") -> a/b/c ; join(path, "b","c") -> path/b/c
    std::filesystem::path join(const std::string& first, std::initializer_list<std::string> others);
    std::filesystem::path join(const std::filesystem::path& first, std::initializer_list<std::string> others);

    /// serialize
    template <typename T>
    std::vector<unsigned char> serialize(const T& obj);

    template <typename T>
    T readObject(const std::filesystem::path& file);

    template <typename T>
    void writeObject(const std::filesystem::path& file, const T& obj);


    void message(const std::string& s);

    /// error() — build a GitcppException with a message.
    GitcppException error(const std::string& s);

    // Template

    namespace detail {

        // type traits: treat const char* as string data
        inline void sha1_update_bytes(std::vector<unsigned char>& sink, const std::vector<unsigned char>& v) {
            sink.insert(sink.end(), v.begin(), v.end());
        }
        inline void sha1_update_bytes(std::vector<unsigned char>& sink, const std::string& s) {
            sink.insert(sink.end(), reinterpret_cast<const unsigned char*>(s.data()),
                        reinterpret_cast<const unsigned char*>(s.data()) + s.size());
        }
        inline void sha1_update_bytes(std::vector<unsigned char>& sink, const char* cstr) {
            if (!cstr) return;
            const auto* p = reinterpret_cast<const unsigned char*>(cstr);
            while (*p) { sink.push_back(*p); ++p; }
        }

        template <typename T>
        auto has_to_bytes_impl(int) -> decltype(std::declval<const T&>().to_bytes(), std::true_type{});
        template <typename>
        std::false_type has_to_bytes_impl(...);

        template <typename T>
        using has_to_bytes = decltype(has_to_bytes_impl<T>(0));

    } 

    template <typename... Args>
    std::string sha1_concat(const Args&... parts) {
        std::vector<unsigned char> all;
        all.reserve(1024);
        (detail::sha1_update_bytes(all, parts), ...);
        return sha1(all);
    }

    template <typename T>
    std::vector<unsigned char> serialize(const T& obj) {
        static_assert(detail::has_to_bytes<T>::value, "Type must define to_bytes() const -> std::vector<unsigned char>");
        return obj.to_bytes();
    }

    template <typename T>
    T readObject(const std::filesystem::path& file) {
        auto bytes = readContents(file);
        // Expect T::from_bytes(bytes)
        return T::from_bytes(bytes);
    }

    template <typename T>
    void writeObject(const std::filesystem::path& file, const T& obj) {
        auto bytes = serialize(obj);
        writeContents(file, bytes);
    }

    template <typename... Args>
    void writeContents(const std::filesystem::path& file, const Args&... parts) {
        // Implemented in Utils.cpp via helper that takes a vector of byte chunks.
        std::vector<std::vector<unsigned char>> chunks;
        chunks.reserve(sizeof...(Args));

        auto push_chunk = [&](const auto& piece) {
            using P = std::decay_t<decltype(piece)>;
            if constexpr (std::is_same_v<P, std::vector<unsigned char>>) {
                chunks.emplace_back(piece);
            } else if constexpr (std::is_same_v<P, std::string>) {
                chunks.emplace_back(reinterpret_cast<const unsigned char*>(piece.data()),
                                    reinterpret_cast<const unsigned char*>(piece.data()) + piece.size());
            } else if constexpr (std::is_same_v<P, const char*>) {
                const char* s = piece ? piece : "";
                chunks.emplace_back(reinterpret_cast<const unsigned char*>(s),
                                    reinterpret_cast<const unsigned char*>(s) + std::char_traits<char>::length(s));
            } else {
                static_assert(!sizeof(P), "writeContents() only accepts std::vector<unsigned char>, std::string, or const char*");
            }
        };

        (push_chunk(parts), ...);

        // call the non-template sink (defined in Utils.cpp)
        extern void writeContents_impl_(const std::filesystem::path& file,
                                        const std::vector<std::vector<unsigned char>>& chunks);
        writeContents_impl_(file, chunks);
    }

} // namespace gitcpp
