#include "Repository.hpp"
#include <fstream>
#include <iostream>

using namespace std;

namespace gitcpp {

    void Repository::ensure_dir(const fs::path& p) {
        if (!fs::exists(p)) {
            fs::create_directories(p);
        }
    }

    void Repository::write_text(const fs::path& p, const std::string& s) {
        std::ofstream ofs(p, ios::trunc | ios::out);
        ofs << s;
    }

    void Repository::write_empty_map(const fs::path& p) { write_text(p, "{}"); }
    void Repository::write_empty_set(const fs::path& p) { write_text(p, "[]"); }

    Repository::Repository() {
        // Use current working directory at time of construction
        CWD = fs::current_path();
        GITCPP_DIR = CWD / ".gitcpp";
        STAGED_FILES = GITCPP_DIR / "staged_files";
        BLOBS = GITCPP_DIR / "blob_files";
        COMMITS = GITCPP_DIR / "commits";
        HEADS = GITCPP_DIR / "heads";
        BRANCHES = GITCPP_DIR / "branches";
        
        if (fs::exists(GITCPP_DIR)) {
            std::cout << "A gitcpp version-control system already exists in the current directory.\n";
            std::exit(0);
        }

        // Create directories
        ensure_dir(GITCPP_DIR);
        ensure_dir(STAGED_FILES);
        ensure_dir(BLOBS);
        ensure_dir(COMMITS);
        ensure_dir(HEADS);
        ensure_dir(BRANCHES);

        write_text(COMMITS / "main", "");          // ""
        write_text(BLOBS / "blob_count", "0");          // "0"
        write_text(BRANCHES / "first_branch_com", "false");// "false"
        write_empty_map(STAGED_FILES / "file_map");            // HashMap -> "{}"
        write_empty_map(STAGED_FILES / "blob_map");    // HashMap -> "{}"
        write_empty_set(BRANCHES / "branch_set");          // HashSet -> "[]"
        write_empty_set(STAGED_FILES / "remove_set");          // HashSet -> "[]"
        write_text(BRANCHES / "current_branch", "main");   // "main"
    }

    Repository::Repository(bool force_init) {
        // Use current working directory at time of construction
        CWD = fs::current_path();
        GITCPP_DIR = CWD / ".gitcpp";
        STAGED_FILES = GITCPP_DIR / "staged_files";
        BLOBS = GITCPP_DIR / "blob_files";
        COMMITS = GITCPP_DIR / "commits";
        HEADS = GITCPP_DIR / "heads";
        BRANCHES = GITCPP_DIR / "branches";
        
        if (fs::exists(GITCPP_DIR) && !force_init) {
            std::cout << "A gitcpp version-control system already exists in the current directory.\n";
            std::exit(0);
        }

        // Remove existing directory if force_init is true
        if (force_init && fs::exists(GITCPP_DIR)) {
            fs::remove_all(GITCPP_DIR);
        }

        // Create directories
        ensure_dir(GITCPP_DIR);
        ensure_dir(STAGED_FILES);
        ensure_dir(BLOBS);
        ensure_dir(COMMITS);
        ensure_dir(HEADS);
        ensure_dir(BRANCHES);

        write_text(COMMITS / "main", "");          // ""
        write_text(BLOBS / "blob_count", "0");          // "0"
        write_text(BRANCHES / "first_branch_com", "false");// "false"
        write_empty_map(STAGED_FILES / "file_map");            // HashMap -> "{}"
        write_empty_map(STAGED_FILES / "blob_map");    // HashMap -> "{}"
        write_empty_set(BRANCHES / "branch_set");          // HashSet -> "[]"
        write_empty_set(STAGED_FILES / "remove_set");          // HashSet -> "[]"
        write_text(BRANCHES / "current_branch", "main");   // "main"
    }

} // namespace gitcpp

