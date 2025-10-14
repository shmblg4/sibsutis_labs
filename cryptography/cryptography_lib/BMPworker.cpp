#include "cryptography.hpp"

std::vector<ll> BMPworker::read(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("File not found: " + filename);
    }

    header.clear();
    std::vector<ll> data;
    byte_t byte_val;

    for (int i = 0; i < BMP_H_SIZE; i++) {
        if (file.read(reinterpret_cast<char *>(&byte_val), 1)) {
            header.push_back(static_cast<ll>(byte_val));
        }
    }

    while (file.read(reinterpret_cast<char *>(&byte_val), 1)) {
        data.push_back(static_cast<ll>(byte_val));
    }

    file.close();
    return data;
}

void BMPworker::write(std::vector<ll> data, std::string filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create file: " + filename);
    }

    for (size_t i = 0; i < header.size(); i++) {
        byte_t byte_val = static_cast<byte_t>(header[i] & 0xFF);
        file.write(reinterpret_cast<char *>(&byte_val), 1);
    }

    for (size_t i = 0; i < data.size(); i++) {
        byte_t byte_val = static_cast<byte_t>(data[i] & 0xFF);
        file.write(reinterpret_cast<char *>(&byte_val), 1);
    }

    file.close();
}