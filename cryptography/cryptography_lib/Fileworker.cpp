#include "cryptography.hpp"

std::vector<ll> Fileworker::read(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("File not found: " + filename);
    }
    std::vector<ll> data;
    byte_t byte_val;
    while (file.read(reinterpret_cast<char *>(&byte_val), 1)) {
        data.push_back(static_cast<ll>(byte_val));
    }
    file.close();
    return data;
}

void Fileworker::write(std::vector<ll> data, std::string filename, int mode) {
    FILE *file = fopen(filename.c_str(), "wb");
    if (!file) {
        throw std::runtime_error("Cannot create file: " + filename);
    }
    if (mode == 1) {
        for (auto val : data) {
            fwrite(&val, sizeof(ll), 1, file);
        }
    } else if (mode == 2) {
        for (auto val : data) {
            fwrite(reinterpret_cast<char *>(&val), sizeof(char), 1, file);
        }
    } else {
        fclose(file);
        return;
    }
    fclose(file);
}