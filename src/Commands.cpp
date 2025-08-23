#include "gitlet/Commands.hpp"
#include "gitlet/Repository.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace gitlet::commands {

// ---- helper (local) ----
static void not_impl(const char* name) {
    std::cout << "[TODO] Command not implemented yet: " << name << "\n";
    std::exit(0);
}

// ---- implemented now ----
void init() {
    // mirrors: new Repository();
    Repository repo; // constructor performs the init and exits if it already exists
}

// ---- stubs (compile-safe) ----
void add(const std::string& fileToAdd) { not_impl("add " + fileToAdd); }
void commit(const std::string& message) { not_impl("commit \"" + message + "\""); }
void remove(const std::string& fileToRemove) { not_impl("rm " + fileToRemove); }
void log() { not_impl("log"); }
void globalLog() { not_impl("global-log"); }
void find(const std::string& message) { not_impl("find \"" + message + "\""); }
void status() { not_impl("status"); }
void restore(const std::vector<std::string>& argv) { (void)argv; not_impl("restore"); }
void branch(const std::string& name) { not_impl("branch " + name); }
void switchBranch(const std::string& name, const std::string& mode) { (void)mode; not_impl(("switch " + name).c_str()); }
void rmBranch(const std::string& name) { not_impl("rm-branch " + name); }
void reset(const std::string& commitId) { (void)commitId; not_impl("reset"); }
void merge(const std::string& otherBranch) { not_impl("merge " + otherBranch); }

// Utility placeholders—wire these up as you implement the internals.
bool isStageEmpty() { return true; }
bool isFirstBranchCom() { return false; }
std::string getHeadPath() { return ""; }
std::string getCurrentBranch() { return "main"; }

} // namespace gitlet::commands
