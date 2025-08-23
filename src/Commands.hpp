#pragma once
#include <string>
#include <vector>

namespace gitlet::commands {

// === Implemented ===
void init();  // uses Repository to create .gitlet

// === Stubs you can fill in next ===
void add(const std::string& fileToAdd);
void commit(const std::string& message);
void remove(const std::string& fileToRemove);
void log();
void globalLog();
void find(const std::string& message);
void status();
void restore(const std::vector<std::string>& argv);         // mirrors restore(args)
void branch(const std::string& name);
void switchBranch(const std::string& name, const std::string& mode);
void rmBranch(const std::string& name);
void reset(const std::string& commitId);                     // switchBranch(commitId, "commit")
void merge(const std::string& otherBranch);


bool isStageEmpty();
bool isFirstBranchCom();
std::string getHeadPath();
std::string getCurrentBranch();

} // namespace gitlet::commands
