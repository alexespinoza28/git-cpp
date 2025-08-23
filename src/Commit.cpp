#include "Commit.hpp"
#include "Utils.hpp"

#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <string>

static std::string getenv_or(const char* key, const char* fallback) {
    if (const char* v = std::getenv(key)) return std::string(v);
    return std::string(fallback);
}


static std::string tz_offset_string(time_t now) {
    tm local_tm{};
    localtime_r(&now, &local_tm);
    long off = local_tm.tm_gmtoff; // seconds from UTC
    char sign = off >= 0 ? '+' : '-';
    off = std::labs(off);
    int hh = static_cast<int>(off / 3600);
    int mm = static_cast<int>((off % 3600) / 60);
    std::ostringstream oss;
    oss << sign << std::setw(2) << std::setfill('0') << hh
        << std::setw(2) << std::setfill('0') << mm;
    return oss.str();
}

Commit::Commit(const std::string& treeHash,
               const std::vector<std::string>& parentHashes,
               const std::string& message)
    : treeHash(treeHash),
      parentHashes(parentHashes),
      message(message)
{
    timestamp = std::time(nullptr);
    std::string tz = tz_offset_string(timestamp);

    std::ostringstream contents;
    contents << "tree " << treeHash << "\n";
    for (const auto& p : parentHashes) contents << "parent " << p << "\n";
    contents << "author " << author << " " << timestamp << " " << tz << "\n";
    contents << "committer " << committer << " " << timestamp << " " << tz << "\n\n";
    contents << message << "\n";

    const std::string body = contents.str();
    commitHash = gitlet::sha1_concat("commit ", std::to_string(body.size()), std::string(1, '\0'), body);
}

const std::string& Commit::getTreeHash() const { return treeHash; }
const std::vector<std::string>& Commit::getParentHashes() const { return parentHashes; }
const std::string& Commit::getAuthor() const { return author; }
const std::string& Commit::getCommitter() const { return committer; }
const std::string& Commit::getMessage() const { return message; }
const std::string& Commit::getCommitHash() const { return commitHash; }
std::time_t Commit::getTimestamp() const { return timestamp; }