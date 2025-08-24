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
    std::string removed_content = gitlet::readContentsAsString(Repository::REMOVE_SET);
    
    // Check if there are any changes to commit (staged files or removed files)
    bool has_staged_files = !(staged_content.empty() || staged_content == "{}");
    bool has_removed_files = !(removed_content.empty() || removed_content == "[]");
    
    if (!has_staged_files && !has_removed_files) {
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

    // Clear staging area and remove set
    gitlet::writeContents(Repository::FILE_MAP, "{}");
    gitlet::writeContents(Repository::REMOVE_SET, "[]");
}

void remove(const std::string& fileToRemove) {
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

    bool removed = false;
    if (staged_files.count(fileToRemove)) {
        staged_files.erase(fileToRemove);
        removed = true;
    }

    // If the file was staged, write the updated index back to disk
    if (removed) {
        std::ostringstream new_index_content;
        for (const auto& [file, hash] : staged_files) {
            new_index_content << file << ":" << hash << "\n";
        }
        gitlet::writeContents(Repository::FILE_MAP, new_index_content.str());
        return;
    }

    // Check if the file is in the current commit
    std::string current_branch = gitlet::readContentsAsString(Repository::CURRENT_BRANCH);
    fs::path head_path = Repository::HEADS / current_branch;
    if (fs::exists(head_path)) {
        std::string head_commit_hash = gitlet::readContentsAsString(head_path);
        fs::path commit_path = Repository::COMMITS / head_commit_hash;
        if (fs::exists(commit_path)) {
            std::string commit_contents = gitlet::readContentsAsString(commit_path);
            size_t nul_pos = commit_contents.find('\0');
            if (nul_pos != std::string::npos) {
                std::string body = commit_contents.substr(nul_pos + 1);
                std::istringstream body_stream(body);
                std::string line;
                std::string tree_hash;
                while (std::getline(body_stream, line)) {
                    if (line.rfind("tree ", 0) == 0) {
                        tree_hash = line.substr(5);
                        break;
                    }
                }

                if (!tree_hash.empty()) {
                    std::string tree_contents = gitlet::readContentsAsString(Repository::BLOBS / tree_hash);
                    if (tree_contents.find(fileToRemove) != std::string::npos) {
                        // Stage for removal
                        std::string removed_content = gitlet::readContentsAsString(Repository::REMOVE_SET);
                        if (removed_content == "[]") {
                            removed_content = "";
                        }
                        if (removed_content.find(fileToRemove) == std::string::npos) {
                            removed_content += fileToRemove + "\n";
                            gitlet::writeContents(Repository::REMOVE_SET, removed_content);
                        }
                        // Delete the file
                        fs::remove(fileToRemove);
                        removed = true;
                    }
                }
            }
        }
    }

    if (!removed) {
        gitlet::message("No reason to remove the file.");
    }
}

void log() {
    std::string current_branch = gitlet::readContentsAsString(Repository::CURRENT_BRANCH);
    fs::path head_path = Repository::HEADS / current_branch;
    if (!fs::exists(head_path)) {
        return; // No commits yet
    }
    std::string current_commit_hash = gitlet::readContentsAsString(head_path);

    while (!current_commit_hash.empty()) {
        fs::path commit_path = Repository::COMMITS / current_commit_hash;
        if (!fs::exists(commit_path)) {
            std::cerr << "Error: Corrupt repository. Commit object not found: " << current_commit_hash << std::endl;
            break;
        }

        std::string commit_contents = gitlet::readContentsAsString(commit_path);
        size_t nul_pos = commit_contents.find('\0');
        if (nul_pos == std::string::npos) {
            std::cerr << "Error: Corrupt repository. Malformed commit object: " << current_commit_hash << std::endl;
            break;
        }

        std::string body = commit_contents.substr(nul_pos + 1);
        std::istringstream body_stream(body);
        std::string line;
        std::string parent_hash = "";

        std::cout << "===" << std::endl;
        std::cout << "commit " << current_commit_hash << std::endl;

        // Parse the commit body header
        while(std::getline(body_stream, line) && !line.empty()) {
            if (line.rfind("parent ", 0) == 0) {
                parent_hash = line.substr(7);
            }
            // A real log would parse the date and format it nicely
            if (line.rfind("author ", 0) == 0) {
                std::cout << line << std::endl;
            }
        }

        // The rest of the stream is the message
        std::string message;
        std::string temp_line;
        while(std::getline(body_stream, temp_line)) {
            message += "    " + temp_line + "\n";
        }
        std::cout << "\n" << message << std::endl;

        current_commit_hash = parent_hash;
    }
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
    fs::path branch_path = Repository::HEADS / name;
    if (fs::exists(branch_path)) {
        gitlet::message("A branch with that name already exists.");
        return;
    }

    std::string current_branch = gitlet::readContentsAsString(Repository::CURRENT_BRANCH);
    fs::path head_path = Repository::HEADS / current_branch;
    std::string head_commit_hash = gitlet::readContentsAsString(head_path);

    gitlet::writeContents(branch_path, head_commit_hash);
}

void switchBranch(const std::string& name, const std::string& mode) {
    fs::path branch_path = Repository::HEADS / name;
    if (!fs::exists(branch_path)) {
        gitlet::message("A branch with that name does not exist.");
        return;
    }

    // Get the commit hash of the branch to switch to
    std::string branch_commit_hash = gitlet::readContentsAsString(branch_path);

    // Get the tree hash from the commit
    fs::path commit_path = Repository::COMMITS / branch_commit_hash;
    std::string commit_contents = gitlet::readContentsAsString(commit_path);
    size_t nul_pos = commit_contents.find('\0');
    std::string body = commit_contents.substr(nul_pos + 1);
    std::istringstream body_stream(body);
    std::string line;
    std::string tree_hash;
    while (std::getline(body_stream, line)) {
        if (line.rfind("tree ", 0) == 0) {
            tree_hash = line.substr(5);
            break;
        }
    }

    // Get the tree of the current branch
    std::string current_branch = gitlet::readContentsAsString(Repository::CURRENT_BRANCH);
    fs::path head_path = Repository::HEADS / current_branch;
    std::string head_commit_hash = gitlet::readContentsAsString(head_path);
    fs::path current_commit_path = Repository::COMMITS / head_commit_hash;
    std::string current_commit_contents = gitlet::readContentsAsString(current_commit_path);
    size_t current_nul_pos = current_commit_contents.find('\0');
    std::string current_body = current_commit_contents.substr(current_nul_pos + 1);
    std::istringstream current_body_stream(current_body);
    std::string current_line;
    std::string current_tree_hash;
    while (std::getline(current_body_stream, current_line)) {
        if (current_line.rfind("tree ", 0) == 0) {
            current_tree_hash = current_line.substr(5);
            break;
        }
    }

    // Delete files that are tracked in the current branch
    std::string current_tree_contents = gitlet::readContentsAsString(Repository::BLOBS / current_tree_hash);
    std::istringstream current_tree_stream(current_tree_contents);
    while (std::getline(current_tree_stream, current_line)) {
        size_t colon_pos = current_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string file_path = current_line.substr(0, colon_pos);
            fs::remove(file_path);
        }
    }

    // Checkout the files from the new branch's tree
    std::string tree_contents = gitlet::readContentsAsString(Repository::BLOBS / tree_hash);
    std::istringstream tree_stream(tree_contents);
    while (std::getline(tree_stream, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string file_path = line.substr(0, colon_pos);
            std::string blob_hash = line.substr(colon_pos + 1);
            std::string blob_contents = gitlet::readContentsAsString(Repository::BLOBS / blob_hash);
            gitlet::writeContents(file_path, blob_contents);
        }
    }

    // Update the current branch
    gitlet::writeContents(Repository::CURRENT_BRANCH, name);
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