#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "Commands.hpp"
#include "Repository.hpp"
#include "Utils.hpp"

namespace fs = std::filesystem;

class BasicOperationsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test directory
        test_dir = fs::temp_directory_path() / "gitcpp_test";
        fs::remove_all(test_dir);
        fs::create_directories(test_dir);
        fs::current_path(test_dir);
    }

    void TearDown() override {
        fs::current_path(fs::temp_directory_path());
        fs::remove_all(test_dir);
    }

    fs::path test_dir;
};

TEST_F(BasicOperationsTest, InitCreatesRepository) {
    gitcpp::Repository repo(true);  // Force init for testing
    
    EXPECT_TRUE(fs::exists(".gitcpp"));
    EXPECT_TRUE(fs::exists(".gitcpp/commits"));
    EXPECT_TRUE(fs::exists(".gitcpp/heads"));
    EXPECT_TRUE(fs::exists(".gitcpp/staged_files"));
}

TEST_F(BasicOperationsTest, AddAndCommitFile) {
    gitcpp::Repository repo(true);  // Force init for testing
    
    // Create a test file
    std::ofstream file("test.txt");
    file << "Hello World";
    file.close();
    
    // Add and commit
    gitcpp::commands::add("test.txt");
    gitcpp::commands::commit("Initial commit");
    
    // Check that commit was created
    EXPECT_TRUE(fs::exists(".gitcpp/commits"));
    auto commits = gitcpp::plainFilenamesIn(".gitcpp/commits");
    EXPECT_EQ(commits.size(), 1);
}

TEST_F(BasicOperationsTest, ConfigStoresValues) {
    gitcpp::Repository repo(true);  // Force init for testing
    
    gitcpp::commands::config("user.name", "Test User");
    gitcpp::commands::config("user.email", "test@example.com");
    
    EXPECT_TRUE(fs::exists(".gitcpp/config/user.name"));
    EXPECT_TRUE(fs::exists(".gitcpp/config/user.email"));
    
    std::string name = gitcpp::readContentsAsString(".gitcpp/config/user.name");
    std::string email = gitcpp::readContentsAsString(".gitcpp/config/user.email");
    
    EXPECT_EQ(name, "Test User");
    EXPECT_EQ(email, "test@example.com");
}