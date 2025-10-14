#include "cryptography.hpp"
#include <iomanip>

std::vector<ll> RSAcoder::encode(std::vector<ll> data) {
    std::vector<ll> encoded_data;
    for (size_t i = 0; i < data.size(); i++) {
        encoded_data.push_back(pow_mod(data[i], d, n));
    }
    return encoded_data;
}

std::vector<ll> RSAcoder::decode(std::vector<ll> data) {
    std::vector<ll> decoded_data;
    for (size_t i = 0; i < data.size(); i++) {
        decoded_data.push_back(pow_mod(data[i], c, n));
    }
    return decoded_data;
}

void RSAcoder::configure() {
    p = gen_prime();
    q = gen_prime();
    while (p == q) {
        q = gen_prime();
    }

    n = p * q;
    phi = (p - 1) * (q - 1);
    gen_keys();

    while ((c * d) % phi != 1) {
        gen_keys();
    }
}

void RSAcoder::gen_keys() {
    d = gen_random();
    while (d < 2 || d >= phi || gcd(d, phi) != 1) {
        d = gen_random();
    }

    std::vector<ll> res = algorithm_Euclid(phi, d);
    c = res[2];
    c = (c % phi + phi) % phi;
}