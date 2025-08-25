#pragma once
#include <stdexcept>
#include <string>

class GitcppException : public std::runtime_error {
public:
    explicit GitcppException(const std::string& msg)
        : std::runtime_error(msg) {}
};