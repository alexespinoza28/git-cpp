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
    #include <set>
    #include <queue>
    
    namespace fs = std::filesystem;
    
namespace gitcpp::commands {
    
    // Helper for commands that are not yet implemented.
    static void not_impl(const char* name) {
        std::cout << "[TODO] Command not implemented yet: " << name << "\n";
        std::exit(0);
    }
    
    // Forward declarations for merge helper functions
    std::string findMergeBase(const std::string& commit1, const std::string& commit2);
    std::set<std::string> getCommitAncestors(const std::string& commitHash);
    void performFastForwardMerge(const std::string& targetCommit, const std::string& branchName);
    void updateWorkingDirectory(const std::string& commitHash);
    void performThreeWayMerge(const std::string& currentCommit, const std::string& otherCommit, 
                             const std::string& baseCommit, const std::string& branchName);
    std::map<std::string, std::string> getFilesFromCommit(const std::string& commitHash);
    std::string mergeFile(const std::string& filePath, const std::string& currentHash, 
                         const std::string& otherHash, const std::string& baseHash, bool& hasConflicts);
    void createConflictFile(const std::string& filePath, const std::string& currentHash, const std::string& otherHash);
    void createMergeCommit(const std::map<std::string, std::string>& files, 
                          const std::string& parent1, const std::string& parent2, const std::string& branchName);
    
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
        auto content_bytes = gitcpp::readContents(file_path);
        std::string blob_hash = gitcpp::sha1(content_bytes);
    
        // Read the staging index
        std::map<std::string, std::string> staged_files;
        std::string index_content = gitcpp::readContentsAsString(Repository::FILE_MAP);
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
        gitcpp::writeContents(Repository::BLOBS / blob_hash, content_bytes);
    
        // Write the staging index back to disk
        std::ostringstream new_index_content;
        for (const auto& [file, hash] : staged_files) {
            new_index_content << file << ":" << hash << "\n";
        }
        gitcpp::writeContents(Repository::FILE_MAP, new_index_content.str());
    }
    
    void commit(const std::string& message) {
        std::string staged_content = gitcpp::readContentsAsString(Repository::FILE_MAP);
        std::string removed_content = gitcpp::readContentsAsString(Repository::REMOVE_SET);
        
        // Check if there are any changes to commit (staged files or removed files)
        bool has_staged_files = !(staged_content.empty() || staged_content == "{}");
        bool has_removed_files = !(removed_content.empty() || removed_content == "[]");
        
        if (!has_staged_files && !has_removed_files) {
            gitcpp::message("Nothing to commit, working tree clean");
            return;
        }
    
        // Create tree hash from the staging index and save the tree
        std::string treeHash = gitcpp::sha1(staged_content);
        gitcpp::writeContents(Repository::BLOBS / treeHash, staged_content);
    
        // Get parent commit hash
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        fs::path head_path = Repository::HEADS / current_branch;
        std::string parent_hash;
        if (fs::exists(head_path)) {
            parent_hash = gitcpp::readContentsAsString(head_path);
        }
        
        std::vector<std::string> parent_hashes;
        if (!parent_hash.empty()) {
            parent_hashes.push_back(parent_hash);
        }
    
        // Create and save commit object
        Commit new_commit(treeHash, parent_hashes, message);
        gitcpp::writeContents(Repository::COMMITS / new_commit.getCommitHash(), new_commit.getCommitContents());
    
        // Update branch head
        gitcpp::writeContents(head_path, new_commit.getCommitHash());
    
        // Clear staging area and remove set
        gitcpp::writeContents(Repository::FILE_MAP, "{}");
        gitcpp::writeContents(Repository::REMOVE_SET, "[]");
    }
    
    void remove(const std::string& fileToRemove) {
        // Read the staging index
        std::map<std::string, std::string> staged_files;
        std::string index_content = gitcpp::readContentsAsString(Repository::FILE_MAP);
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
            gitcpp::writeContents(Repository::FILE_MAP, new_index_content.str());
            return;
        }
    
        // Check if the file is in the current commit
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        fs::path head_path = Repository::HEADS / current_branch;
        if (fs::exists(head_path)) {
            std::string head_commit_hash = gitcpp::readContentsAsString(head_path);
            fs::path commit_path = Repository::COMMITS / head_commit_hash;
            if (fs::exists(commit_path)) {
                std::string commit_contents = gitcpp::readContentsAsString(commit_path);
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
                        std::string tree_contents = gitcpp::readContentsAsString(Repository::BLOBS / tree_hash);
                        if (tree_contents.find(fileToRemove) != std::string::npos) {
                            // Stage for removal
                            std::string removed_content = gitcpp::readContentsAsString(Repository::REMOVE_SET);
                            if (removed_content == "[]") {
                                removed_content = "";
                            }
                            if (removed_content.find(fileToRemove) == std::string::npos) {
                                removed_content += fileToRemove + "\n";
                                gitcpp::writeContents(Repository::REMOVE_SET, removed_content);
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
            gitcpp::message("No reason to remove the file.");
        }
    }
    
    void log() {
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        fs::path head_path = Repository::HEADS / current_branch;
        if (!fs::exists(head_path)) {
            return; // No commits yet
        }
        std::string current_commit_hash = gitcpp::readContentsAsString(head_path);
    
        while (!current_commit_hash.empty()) {
            fs::path commit_path = Repository::COMMITS / current_commit_hash;
            if (!fs::exists(commit_path)) {
                std::cerr << "Error: Corrupt repository. Commit object not found: " << current_commit_hash << std::endl;
                break;
            }
    
            std::string commit_contents = gitcpp::readContentsAsString(commit_path);
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
        // Get all commit files from the commits directory
        std::vector<std::string> all_commits = gitcpp::plainFilenamesIn(Repository::COMMITS);
        
        if (all_commits.empty()) {
            return;
        }
        
        // Filter out invalid commit files (SHA-1 hashes should be 40 characters)
        std::vector<std::string> valid_commits;
        for (const std::string& commit_hash : all_commits) {
            if (commit_hash.length() == 40) {
                valid_commits.push_back(commit_hash);
            }
        }
        
        // Sort commits by filename (which are SHA-1 hashes) for consistent output
        std::sort(valid_commits.begin(), valid_commits.end());
        
        for (const std::string& commit_hash : valid_commits) {
            fs::path commit_path = Repository::COMMITS / commit_hash;
            
            if (!fs::exists(commit_path)) {
                continue;
            }
            
            std::string commit_contents = gitcpp::readContentsAsString(commit_path);
            size_t nul_pos = commit_contents.find('\0');
            if (nul_pos == std::string::npos) {
                std::cerr << "Error: Corrupt repository. Malformed commit object: " << commit_hash << std::endl;
                continue;
            }
            
            std::string body = commit_contents.substr(nul_pos + 1);
            std::istringstream body_stream(body);
            std::string line;
            
            std::cout << "===" << std::endl;
            std::cout << "commit " << commit_hash << std::endl;
            
            // Parse the commit body header
            while(std::getline(body_stream, line) && !line.empty()) {
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
        }
    }
    
    void find(const std::string& message) {
        // Get all commit files from the commits directory
        std::vector<std::string> all_commits = gitcpp::plainFilenamesIn(Repository::COMMITS);
        
        if (all_commits.empty()) {
            std::cout << "Found no commit with that message." << std::endl;
            return;
        }
        
        // Filter out invalid commit files (SHA-1 hashes should be 40 characters)
        std::vector<std::string> valid_commits;
        for (const std::string& commit_hash : all_commits) {
            if (commit_hash.length() == 40) {
                valid_commits.push_back(commit_hash);
            }
        }
        
        std::vector<std::string> matching_commits;
        
        for (const std::string& commit_hash : valid_commits) {
            fs::path commit_path = Repository::COMMITS / commit_hash;
            
            if (!fs::exists(commit_path)) {
                continue;
            }
            
            std::string commit_contents = gitcpp::readContentsAsString(commit_path);
            size_t nul_pos = commit_contents.find('\0');
            if (nul_pos == std::string::npos) {
                continue; // Skip malformed commits
            }
            
            std::string body = commit_contents.substr(nul_pos + 1);
            std::istringstream body_stream(body);
            std::string line;
            
            // Skip header lines until we get to the message
            while(std::getline(body_stream, line) && !line.empty()) {
                // Skip header lines (tree, parent, author, etc.)
            }
            
            // The rest of the stream is the message
            std::string commit_message;
            std::string temp_line;
            while(std::getline(body_stream, temp_line)) {
                if (!commit_message.empty()) {
                    commit_message += "\n";
                }
                commit_message += temp_line;
            }
            
            // Check if the commit message matches
            if (commit_message == message) {
                matching_commits.push_back(commit_hash);
            }
        }
        
        if (matching_commits.empty()) {
            std::cout << "Found no commit with that message." << std::endl;
        } else {
            // Sort for consistent output
            std::sort(matching_commits.begin(), matching_commits.end());
            for (const std::string& commit_hash : matching_commits) {
                std::cout << commit_hash << std::endl;
            }
        }
    }
    
    void status() {
        std::cout << "=== Branches ===" << std::endl;
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        std::vector<std::string> branches = gitcpp::plainFilenamesIn(Repository::HEADS);
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
        std::string staged_content = gitcpp::readContentsAsString(Repository::FILE_MAP);
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
        std::string removed_content = gitcpp::readContentsAsString(Repository::REMOVE_SET);
        if (removed_content != "[]") {
            std::istringstream removed_stream(removed_content);
            std::string line;
            while (std::getline(removed_stream, line)) {
                std::cout << line << std::endl;
            }
        }
        std::cout << std::endl;
    
        std::cout << "=== Modifications Not Staged For Commit ===" << std::endl;
        
        // Get current commit's tree to compare against
        std::map<std::string, std::string> current_commit_files;
        fs::path head_path = Repository::HEADS / current_branch;
        if (fs::exists(head_path)) {
            std::string head_commit_hash = gitcpp::readContentsAsString(head_path);
            fs::path commit_path = Repository::COMMITS / head_commit_hash;
            if (fs::exists(commit_path)) {
                std::string commit_contents = gitcpp::readContentsAsString(commit_path);
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
                    
                    if (!tree_hash.empty() && fs::exists(Repository::BLOBS / tree_hash)) {
                        std::string tree_contents = gitcpp::readContentsAsString(Repository::BLOBS / tree_hash);
                        std::istringstream tree_stream(tree_contents);
                        while (std::getline(tree_stream, line)) {
                            size_t colon_pos = line.find(':');
                            if (colon_pos != std::string::npos) {
                                current_commit_files[line.substr(0, colon_pos)] = line.substr(colon_pos + 1);
                            }
                        }
                    }
                }
            }
        }
        
        // Get staged files
        std::map<std::string, std::string> staged_files;
        if (staged_content != "{}") {
            std::istringstream staged_stream(staged_content);
            std::string line;
            while (std::getline(staged_stream, line)) {
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    staged_files[line.substr(0, colon_pos)] = line.substr(colon_pos + 1);
                }
            }
        }
        
        // Get removed files
        std::set<std::string> removed_files;
        if (removed_content != "[]") {
            std::istringstream removed_stream(removed_content);
            std::string line;
            while (std::getline(removed_stream, line)) {
                removed_files.insert(line);
            }
        }
        
        // Check for modifications and deletions
        std::vector<std::string> modifications;
        for (const auto& [file_path, blob_hash] : current_commit_files) {
            if (staged_files.count(file_path) || removed_files.count(file_path)) {
                // File is staged or staged for removal, skip checking working directory
                continue;
            }
            
            if (!fs::exists(file_path)) {
                // File was deleted
                modifications.push_back(file_path + " (deleted)");
            } else {
                // Check if file was modified
                auto content_bytes = gitcpp::readContents(file_path);
                std::string current_hash = gitcpp::sha1(content_bytes);
                if (current_hash != blob_hash) {
                    modifications.push_back(file_path + " (modified)");
                }
            }
        }
        
        std::sort(modifications.begin(), modifications.end());
        for (const auto& mod : modifications) {
            std::cout << mod << std::endl;
        }
        std::cout << std::endl;
    
        std::cout << "=== Untracked Files ===" << std::endl;
        
        // Find untracked files (files in working directory not in commit or staging)
        std::vector<std::string> untracked_files;
        std::function<void(const fs::path&)> scan_directory = [&](const fs::path& dir) {
            if (!fs::exists(dir) || !fs::is_directory(dir)) return;
            
            for (const auto& entry : fs::directory_iterator(dir)) {
                std::string filename = entry.path().filename().string();
                if (!filename.empty() && filename[0] == '.') continue; // Skip hidden files
                
                if (entry.is_directory()) {
                    scan_directory(entry.path());
                } else if (entry.is_regular_file()) {
                    std::string relative_path = fs::relative(entry.path(), fs::current_path()).string();
                    
                    // Skip if file is in current commit or staged
                    if (!current_commit_files.count(relative_path) && !staged_files.count(relative_path)) {
                        untracked_files.push_back(relative_path);
                    }
                }
            }
        };
        
        scan_directory(".");
        std::sort(untracked_files.begin(), untracked_files.end());
        for (const auto& file : untracked_files) {
            std::cout << file << std::endl;
        }
        std::cout << std::endl;
    }
    
    void restore(const std::vector<std::string>& argv) {
        // Skip the "restore" command itself
        std::vector<std::string> args;
        for (size_t i = 1; i < argv.size(); ++i) {
            args.push_back(argv[i]);
        }
        
        if (args.empty()) {
            std::cout << "Must specify a file to restore." << std::endl;
            return;
        }
        
        std::string commit_id;
        std::string file_path;
        
        // Parse arguments
        if (args.size() == 1) {
            // restore <file> - restore from HEAD
            file_path = args[0];
            
            // Get current branch's HEAD commit
            std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
            fs::path head_path = Repository::HEADS / current_branch;
            if (!fs::exists(head_path)) {
                std::cout << "No commits yet." << std::endl;
                return;
            }
            commit_id = gitcpp::readContentsAsString(head_path);
        } else if (args.size() == 2 && args[0].rfind("--source=", 0) == 0) {
            // restore --source=<commit> <file>
            commit_id = args[0].substr(9); // Remove "--source=" prefix
            file_path = args[1];
        } else {
            std::cout << "Invalid restore command format." << std::endl;
            return;
        }
        
        // Validate commit exists
        fs::path commit_path = Repository::COMMITS / commit_id;
        if (!fs::exists(commit_path)) {
            std::cout << "No commit with that id exists." << std::endl;
            return;
        }
        
        // Read commit to get tree hash
        std::string commit_contents = gitcpp::readContentsAsString(commit_path);
        size_t nul_pos = commit_contents.find('\0');
        if (nul_pos == std::string::npos) {
            std::cout << "Corrupt commit object." << std::endl;
            return;
        }
        
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
        
        if (tree_hash.empty()) {
            std::cout << "Corrupt commit object - no tree found." << std::endl;
            return;
        }
        
        // Read tree to find file
        fs::path tree_path = Repository::BLOBS / tree_hash;
        if (!fs::exists(tree_path)) {
            std::cout << "Corrupt repository - tree object missing." << std::endl;
            return;
        }
        
        std::string tree_contents = gitcpp::readContentsAsString(tree_path);
        std::istringstream tree_stream(tree_contents);
        std::string blob_hash;
        bool file_found = false;
        
        while (std::getline(tree_stream, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string tree_file_path = line.substr(0, colon_pos);
                if (tree_file_path == file_path) {
                    blob_hash = line.substr(colon_pos + 1);
                    file_found = true;
                    break;
                }
            }
        }
        
        if (!file_found) {
            std::cout << "File does not exist in that commit." << std::endl;
            return;
        }
        
        // Read blob and restore file
        fs::path blob_path = Repository::BLOBS / blob_hash;
        if (!fs::exists(blob_path)) {
            std::cout << "Corrupt repository - blob object missing." << std::endl;
            return;
        }
        
        // Create parent directories if needed
        fs::path file_fs_path(file_path);
        if (file_fs_path.has_parent_path()) {
            fs::create_directories(file_fs_path.parent_path());
        }
        
        // Copy blob content to working directory
        auto blob_contents = gitcpp::readContents(blob_path);
        gitcpp::writeContents(file_fs_path, blob_contents);
        
        std::cout << "Restored " << file_path << " from commit " << commit_id << std::endl;
    }
    
    void branch(const std::string& name) {
        // Create new branch
        fs::path branch_path = Repository::HEADS / name;
        if (fs::exists(branch_path)) {
            gitcpp::message("A branch with that name already exists.");
            return;
        }
    
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        fs::path head_path = Repository::HEADS / current_branch;
        
        // Check if current branch has any commits
        if (!fs::exists(head_path)) {
            gitcpp::message("Cannot create branch before initial commit.");
            return;
        }
        
        std::string head_commit_hash = gitcpp::readContentsAsString(head_path);
        gitcpp::writeContents(branch_path, head_commit_hash);
    }
    
    void switchBranch(const std::string& name, const std::string& mode) {
        fs::path branch_path = Repository::HEADS / name;
        if (!fs::exists(branch_path)) {
            gitcpp::message("A branch with that name does not exist.");
            return;
        }
    
        // Check if we're already on this branch
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        if (current_branch == name) {
            gitcpp::message("Already on '" + name + "'");
            return;
        }
    
        // Get the commit hash of the branch to switch to
        std::string branch_commit_hash = gitcpp::readContentsAsString(branch_path);
    
        // Get the tree hash from the commit
        fs::path commit_path = Repository::COMMITS / branch_commit_hash;
        std::string commit_contents = gitcpp::readContentsAsString(commit_path);
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
        fs::path head_path = Repository::HEADS / current_branch;
        std::string head_commit_hash = gitcpp::readContentsAsString(head_path);
        fs::path current_commit_path = Repository::COMMITS / head_commit_hash;
        std::string current_commit_contents = gitcpp::readContentsAsString(current_commit_path);
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
        std::string current_tree_contents = gitcpp::readContentsAsString(Repository::BLOBS / current_tree_hash);
        std::istringstream current_tree_stream(current_tree_contents);
        while (std::getline(current_tree_stream, current_line)) {
            size_t colon_pos = current_line.find(':');
            if (colon_pos != std::string::npos) {
                std::string file_path = current_line.substr(0, colon_pos);
                fs::remove(file_path);
            }
        }
    
        // Checkout the files from the new branch's tree
        std::string tree_contents = gitcpp::readContentsAsString(Repository::BLOBS / tree_hash);
        std::istringstream tree_stream(tree_contents);
        while (std::getline(tree_stream, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string file_path = line.substr(0, colon_pos);
                std::string blob_hash = line.substr(colon_pos + 1);
                std::string blob_contents = gitcpp::readContentsAsString(Repository::BLOBS / blob_hash);
                
                // Create parent directories if they don't exist
                fs::path parent_dir = fs::path(file_path).parent_path();
                if (!parent_dir.empty() && !fs::exists(parent_dir)) {
                    fs::create_directories(parent_dir);
                }
                
                gitcpp::writeContents(file_path, blob_contents);
            }
        }
    
        // Update the current branch
        gitcpp::writeContents(Repository::CURRENT_BRANCH, name);
    }
    
    void rmBranch(const std::string& name) {
        // Check if branch exists
        fs::path branch_path = Repository::HEADS / name;
        if (!fs::exists(branch_path)) {
            std::cout << "A branch with that name does not exist." << std::endl;
            return;
        }
        
        // Check if trying to delete the current branch
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        if (name == current_branch) {
            std::cout << "Cannot remove the current branch." << std::endl;
            return;
        }
        
        // Remove the branch file
        try {
            fs::remove(branch_path);
            std::cout << "Deleted branch " << name << "." << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error removing branch: " << e.what() << std::endl;
        }
    }
    
    void reset(const std::string& commitId) {
        // Validate commit exists
        fs::path commit_path = Repository::COMMITS / commitId;
        if (!fs::exists(commit_path)) {
            std::cout << "No commit with that id exists." << std::endl;
            return;
        }
        
        // Read commit to get tree hash
        std::string commit_contents = gitcpp::readContentsAsString(commit_path);
        size_t nul_pos = commit_contents.find('\0');
        if (nul_pos == std::string::npos) {
            std::cout << "Corrupt commit object." << std::endl;
            return;
        }
        
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
        
        if (tree_hash.empty()) {
            std::cout << "Corrupt commit object - no tree found." << std::endl;
            return;
        }
        
        // Read tree to get all files in the commit
        fs::path tree_path = Repository::BLOBS / tree_hash;
        if (!fs::exists(tree_path)) {
            std::cout << "Corrupt repository - tree object missing." << std::endl;
            return;
        }
        
        std::string tree_contents = gitcpp::readContentsAsString(tree_path);
        std::istringstream tree_stream(tree_contents);
        std::map<std::string, std::string> commit_files;
        
        while (std::getline(tree_stream, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string file_path = line.substr(0, colon_pos);
                std::string blob_hash = line.substr(colon_pos + 1);
                commit_files[file_path] = blob_hash;
            }
        }
        
        // Get current working directory files to know what to remove
        std::set<std::string> current_files;
        std::function<void(const fs::path&)> scan_directory = [&](const fs::path& dir) {
            if (!fs::exists(dir) || !fs::is_directory(dir)) return;
            
            for (const auto& entry : fs::directory_iterator(dir)) {
                std::string filename = entry.path().filename().string();
                if (!filename.empty() && filename[0] == '.') continue; // Skip hidden files
                
                if (entry.is_directory()) {
                    scan_directory(entry.path());
                } else if (entry.is_regular_file()) {
                    std::string relative_path = fs::relative(entry.path(), fs::current_path()).string();
                    current_files.insert(relative_path);
                }
            }
        };
        
        scan_directory(".");
        
        // Remove files that are not in the target commit
        for (const std::string& file_path : current_files) {
            if (commit_files.find(file_path) == commit_files.end()) {
                fs::remove(file_path);
            }
        }
        
        // Restore all files from the commit
        for (const auto& [file_path, blob_hash] : commit_files) {
            fs::path blob_path = Repository::BLOBS / blob_hash;
            if (!fs::exists(blob_path)) {
                std::cout << "Warning: blob object missing for " << file_path << std::endl;
                continue;
            }
            
            // Create parent directories if needed
            fs::path file_fs_path(file_path);
            if (file_fs_path.has_parent_path()) {
                fs::create_directories(file_fs_path.parent_path());
            }
            
            // Copy blob content to working directory
            auto blob_contents = gitcpp::readContents(blob_path);
            gitcpp::writeContents(file_fs_path, blob_contents);
        }
        
        // Update current branch to point to the new commit
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        fs::path head_path = Repository::HEADS / current_branch;
        gitcpp::writeContents(head_path, commitId);
        
        // Clear staging area
        gitcpp::writeContents(Repository::FILE_MAP, "{}");
        gitcpp::writeContents(Repository::REMOVE_SET, "[]");
        
        std::cout << "Reset to commit " << commitId << std::endl;
    }
    
    void merge(const std::string& otherBranch) {
        // Check if other branch exists
        fs::path other_branch_path = Repository::HEADS / otherBranch;
        if (!fs::exists(other_branch_path)) {
            gitcpp::message("A branch with that name does not exist.");
            return;
        }
        
        // Get current branch
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        if (current_branch == otherBranch) {
            gitcpp::message("Cannot merge a branch with itself.");
            return;
        }
        
        // Get commit hashes
        fs::path current_head_path = Repository::HEADS / current_branch;
        std::string current_commit = gitcpp::readContentsAsString(current_head_path);
        std::string other_commit = gitcpp::readContentsAsString(other_branch_path);
        
        // Check if we're already up to date
        if (current_commit == other_commit) {
            gitcpp::message("Already up to date.");
            return;
        }
        
        // Find common ancestor (merge base)
        std::string merge_base = findMergeBase(current_commit, other_commit);
        
        // Check for fast-forward merge
        if (merge_base == current_commit) {
            // Fast-forward merge: just update current branch to other commit
            performFastForwardMerge(other_commit, otherBranch);
            return;
        }
        
        if (merge_base == other_commit) {
            gitcpp::message("Already up to date.");
            return;
        }
        
        // Perform three-way merge
        performThreeWayMerge(current_commit, other_commit, merge_base, otherBranch);
    }
    
    // Utility placeholders
    bool isStageEmpty() { return true; }
    bool isFirstBranchCom() { return false; }
    std::string getHeadPath() { return ""; }
    std::string getCurrentBranch() { return "main"; }
    
    // Helper function implementations for merge
    
    // Get all ancestors of a commit
    std::set<std::string> getCommitAncestors(const std::string& commitHash) {
        std::set<std::string> ancestors;
        std::queue<std::string> toVisit;
        
        if (!commitHash.empty()) {
            toVisit.push(commitHash);
            ancestors.insert(commitHash);
        }
        
        while (!toVisit.empty()) {
            std::string current = toVisit.front();
            toVisit.pop();
            
            fs::path commit_path = Repository::COMMITS / current;
            if (!fs::exists(commit_path)) continue;
            
            std::string commit_contents = gitcpp::readContentsAsString(commit_path);
            size_t nul_pos = commit_contents.find('\0');
            if (nul_pos == std::string::npos) continue;
            
            // Parse commit to find parent
            std::string metadata = commit_contents.substr(nul_pos + 1);
            std::istringstream stream(metadata);
            std::string line;
            
            while (std::getline(stream, line)) {
                if (line.find("parent ") == 0) {
                    std::string parent = line.substr(7); // Skip "parent "
                    if (ancestors.find(parent) == ancestors.end()) {
                        ancestors.insert(parent);
                        toVisit.push(parent);
                    }
                }
            }
        }
        
        return ancestors;
    }
    
    // Helper function to find merge base (common ancestor)
    std::string findMergeBase(const std::string& commit1, const std::string& commit2) {
        // Simple implementation: find first common ancestor
        std::set<std::string> ancestors1 = getCommitAncestors(commit1);
        std::set<std::string> ancestors2 = getCommitAncestors(commit2);
        
        // Find intersection
        for (const auto& ancestor : ancestors1) {
            if (ancestors2.count(ancestor)) {
                return ancestor;
            }
        }
        
        return ""; // No common ancestor found
    }
    
    // Update working directory to match a commit
    void updateWorkingDirectory(const std::string& commitHash) {
        fs::path commit_path = Repository::COMMITS / commitHash;
        if (!fs::exists(commit_path)) return;
        
        std::string commit_contents = gitcpp::readContentsAsString(commit_path);
        size_t nul_pos = commit_contents.find('\0');
        if (nul_pos == std::string::npos) return;
        
        std::string tree_hash = commit_contents.substr(0, nul_pos);
        if (tree_hash.empty()) return;
        
        fs::path tree_path = Repository::BLOBS / tree_hash;
        if (!fs::exists(tree_path)) return;
        
        std::string tree_contents = gitcpp::readContentsAsString(tree_path);
        std::istringstream tree_stream(tree_contents);
        std::string line;
        
        // Clear current working directory of tracked files
        // (In a real implementation, you'd be more careful about this)
        
        // Restore files from the commit
        while (std::getline(tree_stream, line)) {
            if (line.empty()) continue;
            
            size_t colon_pos = line.find(':');
            if (colon_pos == std::string::npos) continue;
            
            std::string file_path = line.substr(0, colon_pos);
            std::string blob_hash = line.substr(colon_pos + 1);
            
            fs::path blob_path = Repository::BLOBS / blob_hash;
            if (fs::exists(blob_path)) {
                auto blob_contents = gitcpp::readContents(blob_path);
                gitcpp::writeContents(file_path, blob_contents);
            }
        }
    }
    
    // Perform fast-forward merge
    void performFastForwardMerge(const std::string& targetCommit, const std::string& branchName) {
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        fs::path current_head_path = Repository::HEADS / current_branch;
        
        // Update current branch to point to target commit
        gitcpp::writeContents(current_head_path, targetCommit);
        
        // Update working directory to match target commit
        updateWorkingDirectory(targetCommit);
        
        std::cout << "Fast-forward merge completed. Merged branch '" << branchName << "' into '" << current_branch << "'." << std::endl;
    }
    
    // Get files from a commit
    std::map<std::string, std::string> getFilesFromCommit(const std::string& commitHash) {
        std::map<std::string, std::string> files;
        
        if (commitHash.empty()) return files;
        
        fs::path commit_path = Repository::COMMITS / commitHash;
        if (!fs::exists(commit_path)) return files;
        
        std::string commit_contents = gitcpp::readContentsAsString(commit_path);
        size_t nul_pos = commit_contents.find('\0');
        if (nul_pos == std::string::npos) return files;
        
        std::string tree_hash = commit_contents.substr(0, nul_pos);
        if (tree_hash.empty()) return files;
        
        fs::path tree_path = Repository::BLOBS / tree_hash;
        if (!fs::exists(tree_path)) return files;
        
        std::string tree_contents = gitcpp::readContentsAsString(tree_path);
        std::istringstream tree_stream(tree_contents);
        std::string line;
        
        while (std::getline(tree_stream, line)) {
            if (line.empty()) continue;
            
            size_t colon_pos = line.find(':');
            if (colon_pos == std::string::npos) continue;
            
            std::string file_path = line.substr(0, colon_pos);
            std::string blob_hash = line.substr(colon_pos + 1);
            files[file_path] = blob_hash;
        }
        
        return files;
    }
    
    // Create a file with conflict markers
    void createConflictFile(const std::string& filePath, const std::string& currentHash, const std::string& otherHash) {
        std::string currentContent = "";
        std::string otherContent = "";
        
        if (!currentHash.empty()) {
            fs::path current_blob = Repository::BLOBS / currentHash;
            if (fs::exists(current_blob)) {
                auto bytes = gitcpp::readContents(current_blob);
                currentContent = std::string(bytes.begin(), bytes.end());
            }
        }
        
        if (!otherHash.empty()) {
            fs::path other_blob = Repository::BLOBS / otherHash;
            if (fs::exists(other_blob)) {
                auto bytes = gitcpp::readContents(other_blob);
                otherContent = std::string(bytes.begin(), bytes.end());
            }
        }
        
        std::string conflictContent = 
            "<<<<<<< HEAD\n" +
            currentContent +
            "\n=======\n" +
            otherContent +
            "\n>>>>>>> " + filePath + "\n";
        
        gitcpp::writeContents(filePath, conflictContent);
    }
    
    // Merge a single file (three-way merge logic)
    std::string mergeFile(const std::string& filePath, const std::string& currentHash, 
                         const std::string& otherHash, const std::string& baseHash, bool& hasConflicts) {
        // Case 1: File unchanged in both branches
        if (currentHash == otherHash) {
            return currentHash;
        }
        
        // Case 2: File only changed in current branch
        if (otherHash == baseHash) {
            return currentHash;
        }
        
        // Case 3: File only changed in other branch
        if (currentHash == baseHash) {
            return otherHash;
        }
        
        // Case 4: File changed in both branches - potential conflict
        if (currentHash != otherHash && currentHash != baseHash && otherHash != baseHash) {
            // For now, mark as conflict and use current version
            // In a full implementation, you'd do line-by-line merging
            std::cout << "CONFLICT (content): Merge conflict in " << filePath << std::endl;
            std::cout << "Automatic merge failed; fix conflicts and then commit the result." << std::endl;
            hasConflicts = true;
            
            // Create conflict markers in the file
            createConflictFile(filePath, currentHash, otherHash);
            return currentHash; // Return current version for now
        }
        
        return currentHash;
    }
    
    // Create merge commit
    void createMergeCommit(const std::map<std::string, std::string>& files, 
                          const std::string& parent1, const std::string& parent2, const std::string& branchName) {
        // Create tree content
        std::ostringstream tree_content;
        for (const auto& [path, hash] : files) {
            tree_content << path << ":" << hash << "\n";
        }
        
        std::string tree_content_str = tree_content.str();
        std::string tree_hash = gitcpp::sha1(tree_content_str);
        gitcpp::writeContents(Repository::BLOBS / tree_hash, tree_content_str);
        
        // Create merge commit with two parents
        std::vector<std::string> parents = {parent1, parent2};
        std::string message = "Merge branch '" + branchName + "'";
        
        Commit merge_commit(tree_hash, parents, message);
        gitcpp::writeContents(Repository::COMMITS / merge_commit.getCommitHash(), merge_commit.getCommitContents());
        
        // Update current branch
        std::string current_branch = gitcpp::readContentsAsString(Repository::CURRENT_BRANCH);
        fs::path current_head_path = Repository::HEADS / current_branch;
        gitcpp::writeContents(current_head_path, merge_commit.getCommitHash());
        
        // Clear staging area
        gitcpp::writeContents(Repository::FILE_MAP, "{}");
        gitcpp::writeContents(Repository::REMOVE_SET, "[]");
        
        std::cout << "Merge completed successfully." << std::endl;
    }
    
    // Perform three-way merge
    void performThreeWayMerge(const std::string& currentCommit, const std::string& otherCommit, 
                             const std::string& baseCommit, const std::string& branchName) {
        // Get file lists from all three commits
        auto currentFiles = getFilesFromCommit(currentCommit);
        auto otherFiles = getFilesFromCommit(otherCommit);
        auto baseFiles = getFilesFromCommit(baseCommit);
        
        // Collect all unique file paths
        std::set<std::string> allFiles;
        for (const auto& [path, hash] : currentFiles) allFiles.insert(path);
        for (const auto& [path, hash] : otherFiles) allFiles.insert(path);
        for (const auto& [path, hash] : baseFiles) allFiles.insert(path);
        
        bool hasConflicts = false;
        std::map<std::string, std::string> mergedFiles;
        
        for (const std::string& filePath : allFiles) {
            std::string currentHash = currentFiles.count(filePath) ? currentFiles[filePath] : "";
            std::string otherHash = otherFiles.count(filePath) ? otherFiles[filePath] : "";
            std::string baseHash = baseFiles.count(filePath) ? baseFiles[filePath] : "";
            
            // Determine merge result for this file
            std::string resultHash = mergeFile(filePath, currentHash, otherHash, baseHash, hasConflicts);
            if (!resultHash.empty()) {
                mergedFiles[filePath] = resultHash;
            }
        }
        
        if (hasConflicts) {
            gitcpp::message("Automatic merge failed; fix conflicts and then commit the result.");
            return;
        }
        
        // Create merge commit
        createMergeCommit(mergedFiles, currentCommit, otherCommit, branchName);
    }
    
} // namespace gitcpp::commands