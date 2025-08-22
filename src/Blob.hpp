// Blob.hpp
#pragma once
#include <string>
#include <vector>
#include <filesystem>

class Blob {
public:
    // Construct from file name 
    Blob(const std::string& fileName, const std::string& blobName);

    // Getters
    const std::string& getFileHash()  const { return fileHash; }
    const std::string& getBlobName()  const { return blobName; }
    const std::string& getFileName()  const { return fileName; }
    const std::filesystem::path& getStoredFile() const { return storedFile; }
    const std::vector<unsigned char>& getFileContents() const { return fileContents; }

private:
    std::string fileName;
    std::vector<unsigned char> fileContents;
    std::string fileHash;                 // SHA-1 hex of fileContents
    std::string blobName;
    std::filesystem::path storedFile;
};