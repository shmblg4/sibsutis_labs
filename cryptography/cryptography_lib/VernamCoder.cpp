#include "cryptography.hpp"

std::vector<ll> VernamCoder::encode(std::vector<ll> data) {
    std::vector<ll> encoded_data;
    for (size_t i = 0; i < data.size(); i++) {
        encoded_data.push_back(data[i] ^ key[i]);
    }
    return encoded_data;
}

std::vector<ll> VernamCoder::decode(std::vector<ll> data) {
    std::vector<ll> decoded_data;
    for (size_t i = 0; i < data.size(); i++) {
        decoded_data.push_back(data[i] ^ key[i]);
    }
    return decoded_data;
}

void VernamCoder::configure() {
    key.resize(data_size);
    for (size_t i = 0; i < data_size; i++) {
        key[i] = rand() % 256;
    }
}