#include "Blob.hpp"
#include "Utils.hpp"

Blob::Blob(const std::string& fileName, const std::string& blobName)
    : fileName(fileName), blobName(blobName)
{
    storedFile = std::filesystem::current_path() / fileName;

    // Read contents and hash them
    fileContents = gitcpp::readContents(storedFile);
    fileHash = gitcpp::sha1(fileContents);
}
