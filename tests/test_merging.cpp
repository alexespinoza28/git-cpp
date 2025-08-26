#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "Commands.hpp"
#include "Repository.hpp"
#include "Utils.hpp"

namespace fs = std::filesystem;

class MergingTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / "gitcpp_merge_test";
        fs::remove_all(test_dir);
        fs::create_directories(test_dir);
        fs::current_path(test_dir);
        
        // Initialize repo with initial commit
        gitcpp::Repository repo(true);  // Force init for testing
        std::ofstream file("shared.txt");
        file << "Line 1: Original\nLine 2: Original\nLine 3: Original";
        file.close();
        gitcpp::commands::add("shared.txt");
        gitcpp::commands::commit("Initial commit");
    }

    void TearDown() override {
        fs::current_path(fs::temp_directory_path());
        fs::remove_all(test_dir);
    }

    fs::path test_dir;
};

TEST_F(MergingTest, FastForwardMerge) {
    // Create feature branch and add commit
    gitcpp::commands::branch("feature");
    gitcpp::commands::switchBranch("feature", "");
    
    std::ofstream file("new_feature.txt");
    file << "New feature file";
    file.close();
    gitcpp::commands::add("new_feature.txt");
    gitcpp::commands::commit("Add new feature");
    
    // Switch back to main and merge
    gitcpp::commands::switchBranch("main", "");
    gitcpp::commands::merge("feature");
    
    // Check that new file exists
    EXPECT_TRUE(fs::exists("new_feature.txt"));
}

TEST_F(MergingTest, ConflictDetection) {
    // Create feature branch
    gitcpp::commands::branch("feature");
    
    // Modify file on main
    std::ofstream main_file("shared.txt");
    main_file << "Line 1: Original\nLine 2: MAIN CHANGE\nLine 3: Original";
    main_file.close();
    gitcpp::commands::add("shared.txt");
    gitcpp::commands::commit("Main branch change");
    
    // Switch to feature and modify same file
    gitcpp::commands::switchBranch("feature", "");
    std::ofstream feature_file("shared.txt");
    feature_file << "Line 1: Original\nLine 2: FEATURE CHANGE\nLine 3: Original";
    feature_file.close();
    gitcpp::commands::add("shared.txt");
    gitcpp::commands::commit("Feature branch change");
    
    // Switch back and attempt merge
    gitcpp::commands::switchBranch("main", "");
    
    // This should create conflict markers
    // Note: In a real test, we'd capture stdout to verify the conflict message
    gitcpp::commands::merge("feature");
    
    // Check that conflict markers exist in file
    std::string content = gitcpp::readContentsAsString("shared.txt");
    EXPECT_TRUE(content.find("<<<<<<< HEAD") != std::string::npos);
    EXPECT_TRUE(content.find("=======") != std::string::npos);
    EXPECT_TRUE(content.find(">>>>>>> shared.txt") != std::string::npos);
}