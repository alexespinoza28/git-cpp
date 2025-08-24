#include "Commands.hpp"
#include "Repository.hpp"
#include "Utils.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace gitlet::commands {

// Helper for commands that are not yet implemented.
static void not_impl(const char* name) {
    std::cout << "[TODO] Command not implemented yet: " << name << "\n";
    std::exit(0);
}

void init() {
    // The constructor handles all the logic for init.
    Repository repo;
}

void add(const std::string& fileToAdd) {
    std::string msg = "add " + fileToAdd;
    not_impl(msg.c_str());
}

void commit(const std::string& message) {
    std::string msg = "commit \"" + message + "\"";
    not_impl(msg.c_str());
}

void remove(const std::string& fileToRemove) {
    std::string msg = "rm " + fileToRemove;
    not_impl(msg.c_str());
}

void log() {
    not_impl("log");
}

void globalLog() {
    not_impl("global-log");
}

void find(const std::string& message) {
    std::string msg = "find \"" + message + "\"";
    not_impl(msg.c_str());
}

void status() {
    std::cout << "=== Branches ===" << std::endl;
    std::string current_branch = gitlet::readContentsAsString(Repository::CURRENT_BRANCH);
    std::vector<std::string> branches = gitlet::plainFilenamesIn(Repository::HEADS);
    if (branches.empty() && !current_branch.empty()) {
        branches.push_back(current_branch);
    }
    std::sort(branches.begin(), branches.end());

    for (const auto& branch : branches) {
        if (branch == current_branch) {
            std::cout << "* " << branch << std::endl;
        } else {
            std::cout << "  " << branch << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "=== Staged Files ===" << std::endl;
    std::string staged_content = gitlet::readContentsAsString(Repository::FILE_MAP);
    if (staged_content != "{}") {
        std::istringstream staged_stream(staged_content);
        std::string line;
        while (std::getline(staged_stream, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::cout << line.substr(0, colon_pos) << std::endl;
            }
        }
    }
    std::cout << std::endl;

    std::cout << "=== Removed Files ===" << std::endl;
    std::string removed_content = gitlet::readContentsAsString(Repository::REMOVE_SET);
    if (removed_content != "[]") {
        std::istringstream removed_stream(removed_content);
        std::string line;
        while (std::getline(removed_stream, line)) {
            std::cout << line << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "=== Modifications Not Staged For Commit ===" << std::endl;
    std::cout << std::endl;

    std::cout << "=== Untracked Files ===" << std::endl;
    std::cout << std::endl;
}

void restore(const std::vector<std::string>& argv) {
    (void)argv; // Unused parameter
    not_impl("restore");
}

void branch(const std::string& name) {
    std::string msg = "branch " + name;
    not_impl(msg.c_str());
}

void switchBranch(const std::string& name, const std::string& mode) {
    (void)mode; // Unused parameter
    std::string msg = "switch " + name;
    not_impl(msg.c_str());
}

void rmBranch(const std::string& name) {
    std::string msg = "rm-branch " + name;
    not_impl(msg.c_str());
}

void reset(const std::string& commitId) {
    (void)commitId; // Unused parameter
    not_impl("reset");
}

void merge(const std::string& otherBranch) {
    std::string msg = "merge " + otherBranch;
    not_impl(msg.c_str());
}

// Utility placeholders
bool isStageEmpty() { return true; }
bool isFirstBranchCom() { return false; }
std::string getHeadPath() { return ""; }
std::string getCurrentBranch() { return "main"; }

} // namespace gitlet::commands