#pragma once
#include <string>
#include <vector>

namespace gitcpp::commands {


    void init();  // uses Repository to create .gitcpp


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
    void config(const std::string& key, const std::string& value);


    // Helper functions for .gitignore support
    bool isIgnored(const std::string& filePath);
    std::vector<std::string> loadGitignorePatterns();

    bool isStageEmpty();
    bool isFirstBranchCom();
    std::string getHeadPath();
    std::string getCurrentBranch();

} // namespace gitcpp::commands
