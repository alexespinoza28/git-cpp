#pragma once
#include <string>
#include <vector>
#include <chrono>

class Commit {
public:
    Commit(const std::string& treeHash,
           const std::vector<std::string>& parentHashes,
           const std::string& message);

    const std::string& getTreeHash() const;
    const std::vector<std::string>& getParentHashes() const;
    const std::string& getAuthor() const;
    const std::string& getCommitter() const;
    const std::string& getMessage() const;
    const std::string& getCommitHash() const;
    std::time_t getTimestamp() const;

private:
    std::string treeHash;
    std::vector<std::string> parentHashes;
    std::string author;
    std::string committer;
    std::string message;
    std::string commitHash;
    std::time_t timestamp;
};
