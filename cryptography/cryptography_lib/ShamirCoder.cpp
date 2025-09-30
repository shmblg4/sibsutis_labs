#include "cryptography.hpp"

ShamirCoder::ShamirCoder() {
    _P = gen_prime();
    gen_keys();
}

std::vector<byte_t> ShamirCoder::worker(std::vector<byte_t> &bytes) {
    std::vector<byte_t> result;
    for (int i = 0; i < bytes.size(); i++) {
        byte_t x1, x2, x3;
        x1 = pow_mod(bytes[i], u1_keys[0], _P);
        x2 = pow_mod(x1, u2_keys[0], _P);
        x3 = pow_mod(x2, u1_keys[1], _P);
        result.push_back(pow_mod(x3, u2_keys[1], _P));
        std::cout << bytes[i] << " -> " << result[i] << std::endl;
    }
    return result;
}

void ShamirCoder::gen_keys() {
    u1_keys[0] = rand() % _P;
    u1_keys[1] = rand() % _P;
    while (u1_keys[0] * u1_keys[1] % (_P - 1) != 1) {
        u1_keys[0] = rand() % _P;
        u1_keys[1] = rand() % _P;
    }
    u2_keys[0] = rand() % _P;
    u2_keys[1] = rand() % _P;
    while (u2_keys[0] * u2_keys[1] % (_P - 1) != 1) {
        u2_keys[0] = rand() % _P;
        u2_keys[1] = rand() % _P;
    }
}