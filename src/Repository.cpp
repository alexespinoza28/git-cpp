#include "Repository.hpp"
#include <fstream>
#include <iostream>

using namespace std;

namespace gitlet {

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
    if (fs::exists(GITLET_DIR)) {
        std::cout << "A Gitlet version-control system already exists in the current directory.\n";
        std::exit(0);
    }

    // Create directories
    ensure_dir(GITLET_DIR);
    ensure_dir(STAGED_FILES);
    ensure_dir(BLOBS);
    ensure_dir(COMMITS);
    ensure_dir(HEADS);
    ensure_dir(BRANCHES);

    // Seed files (mirror your Java defaults)
    write_text(MAIN_COMMIT, "");          // ""
    write_text(BLOB_COUNT, "0");          // "0"
    write_text(FIRST_BRANCH_COM, "false");// "false"
    write_empty_map(FILE_MAP);            // HashMap -> "{}"
    write_empty_map(FILE_TO_BLOB_MAP);    // HashMap -> "{}"
    write_empty_set(BRANCH_SET);          // HashSet -> "[]"
    write_empty_set(REMOVE_SET);          // HashSet -> "[]"
    write_text(CURRENT_BRANCH, "main");   // "main"
}

} // namespace gitlet
