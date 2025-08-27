#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace gitcpp {

    namespace fs = std::filesystem;

    class Repository {
    public:
        fs::path CWD;
        fs::path GITCPP_DIR;
        fs::path COMMITS;
        fs::path STAGED_FILES;
        fs::path BLOBS;
        fs::path FILE_MAP;
        fs::path REMOVE_SET;
        fs::path BLOB_COUNT;
        fs::path HEADS;
        fs::path FILE_TO_BLOB_MAP;
        fs::path MAIN_COMMIT;
        fs::path BRANCHES;
        fs::path BRANCH_SET;
        fs::path FIRST_BRANCH_COM;
        fs::path CURRENT_BRANCH;

        // Constructor = "gitcpp init"
        Repository();
        
        // Constructor with force flag for testing
        Repository(bool force_init);

    private:
        static void ensure_dir(const fs::path& p);
        static void write_text(const fs::path& p, const std::string& s);
        static void write_empty_map(const fs::path& p);   // "{}"
        static void write_empty_set(const fs::path& p);   // "[]"
    };

} // namespace gitcpp
