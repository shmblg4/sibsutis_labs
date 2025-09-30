#include "cryptography.hpp"

std::vector<byte_t> FileWorker::read(std::string filename) {
    if (!filename.empty()) {
        std::ifstream file(filename, std::ios::binary);
        std::vector<byte_t> bytes;
        byte_t byte;
        while (file.read(reinterpret_cast<char *>(&byte), 1)) {
            bytes.push_back(byte);
        }
        file.close();
        return bytes;
    }
    return std::vector<byte_t>();
}

void FileWorker::write(std::vector<byte_t> &bytes, std::string filename) {
    if (!filename.empty()) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        file.close();
    }
}