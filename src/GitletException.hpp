#pragma once
#include <stdexcept>
#include <string>

class GitletException : public std::runtime_error {
public:
    explicit GitletException(const std::string& msg)
        : std::runtime_error(msg) {}
};
