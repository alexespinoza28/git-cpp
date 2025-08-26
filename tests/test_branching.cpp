#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "Commands.hpp"
#include "Repository.hpp"
#include "Utils.hpp"

namespace fs = std::filesystem;

class BranchingTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / "gitcpp_branch_test";
        fs::remove_all(test_dir);
        fs::create_directories(test_dir);
        fs::current_path(test_dir);
        
        // Initialize repo with initial commit
        gitcpp::Repository repo(true);  // Force init for testing
        std::ofstream file("initial.txt");
        file << "Initial content";
        file.close();
        gitcpp::commands::add("initial.txt");
        gitcpp::commands::commit("Initial commit");
    }

    void TearDown() override {
        fs::current_path(fs::temp_directory_path());
        fs::remove_all(test_dir);
    }

    fs::path test_dir;
};

TEST_F(BranchingTest, CreateBranch) {
    gitcpp::commands::branch("feature");
    
    EXPECT_TRUE(fs::exists(".gitcpp/heads/feature"));
    EXPECT_TRUE(fs::exists(".gitcpp/heads/main"));
}

TEST_F(BranchingTest, SwitchBranch) {
    gitcpp::commands::branch("feature");
    gitcpp::commands::switchBranch("feature", "");
    
    std::string current_branch = gitcpp::readContentsAsString(".gitcpp/branches/current_branch");
    EXPECT_EQ(current_branch, "feature");
}

TEST_F(BranchingTest, DeleteBranch) {
    gitcpp::commands::branch("temp_branch");
    EXPECT_TRUE(fs::exists(".gitcpp/heads/temp_branch"));
    
    gitcpp::commands::rmBranch("temp_branch");
    EXPECT_FALSE(fs::exists(".gitcpp/heads/temp_branch"));
}