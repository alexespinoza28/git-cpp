#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace gitlet {

namespace fs = std::filesystem;

class Repository {
public:
    // Mirrors your Java static finals
    inline static const fs::path CWD            = fs::current_path();
    inline static const fs::path GITLET_DIR     = CWD / ".gitlet";
    inline static const fs::path COMMITS        = GITLET_DIR / "commits";
    inline static const fs::path STAGED_FILES   = GITLET_DIR / "staged_files";
    inline static const fs::path BLOBS          = GITLET_DIR / "blob_files";
    inline static const fs::path FILE_MAP       = STAGED_FILES / "file_map";
    inline static const fs::path REMOVE_SET     = STAGED_FILES / "remove_set";
    inline static const fs::path BLOB_COUNT     = BLOBS / "blob_count";
    inline static const fs::path HEADS          = GITLET_DIR / "heads";
    inline static const fs::path FILE_TO_BLOB_MAP = STAGED_FILES / "blob_map";
    inline static const fs::path MAIN_COMMIT    = COMMITS / "main";
    inline static const fs::path BRANCHES       = GITLET_DIR / "branches";
    inline static const fs::path BRANCH_SET     = BRANCHES / "branch_set";
    inline static const fs::path FIRST_BRANCH_COM = BRANCHES / "first_branch_com";
    inline static const fs::path CURRENT_BRANCH = BRANCHES / "current_branch";

    // Constructor = "gitlet init"
    Repository();

private:
    static void ensure_dir(const fs::path& p);
    static void write_text(const fs::path& p, const std::string& s);
    static void write_empty_map(const fs::path& p);   // "{}"
    static void write_empty_set(const fs::path& p);   // "[]"
};

} // namespace gitlet
