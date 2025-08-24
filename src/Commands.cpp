#include "Commands.hpp"
#include "Repository.hpp"
#include "Utils.hpp"
#include "Commit.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

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
    fs::path file_path(fileToAdd);
    if (!fs::exists(file_path)) {
        // Following git's behavior of printing to stderr and exiting with 1
        std::cerr << "Error: File does not exist: " << fileToAdd << std::endl;
        std::exit(1);
    }

    // Read file and create blob
    auto content_bytes = gitlet::readContents(file_path);
    std::string blob_hash = gitlet::sha1(content_bytes);

    // Read the staging index
    std::map<std::string, std::string> staged_files;
    std::string index_content = gitlet::readContentsAsString(Repository::FILE_MAP);
    if (index_content != "{}" && !index_content.empty()) {
        std::istringstream stream(index_content);
        std::string line;
        while (std::getline(stream, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                staged_files[line.substr(0, colon_pos)] = line.substr(colon_pos + 1);
            }
        }
    }

    // Add or update the file in the staging index
    staged_files[fileToAdd] = blob_hash;

    // Write the blob file to the blob store
    gitlet::writeContents(Repository::BLOBS / blob_hash, content_bytes);

    // Write the staging index back to disk
    std::ostringstream new_index_content;
    for (const auto& [file, hash] : staged_files) {
        new_index_content << file << ":" << hash << "\n";
    }
    gitlet::writeContents(Repository::FILE_MAP, new_index_content.str());
}

void commit(const std::string& message) {
    std::string staged_content = gitlet::readContentsAsString(Repository::FILE_MAP);
    if (staged_content.empty() || staged_content == "{}") {
        gitlet::message("Nothing to commit, working tree clean");
        return;
    }

    // Create tree hash from the staging index
    std::string treeHash = gitlet::sha1(staged_content);

    // Get parent commit hash
    std::string current_branch = gitlet::readContentsAsString(Repository::CURRENT_BRANCH);
    fs::path head_path = Repository::HEADS / current_branch;
    std::string parent_hash;
    if (fs::exists(head_path)) {
        parent_hash = gitlet::readContentsAsString(head_path);
    }
    
    std::vector<std::string> parent_hashes;
    if (!parent_hash.empty()) {
        parent_hashes.push_back(parent_hash);
    }

    // Create and save commit object
    Commit new_commit(treeHash, parent_hashes, message);
    gitlet::writeContents(Repository::COMMITS / new_commit.getCommitHash(), new_commit.getCommitContents());

    // Update branch head
    gitlet::writeContents(head_path, new_commit.getCommitHash());

    // Clear staging area
    gitlet::writeContents(Repository::FILE_MAP, "{}");
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
