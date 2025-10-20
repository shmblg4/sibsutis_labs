#include "cryptography.hpp"
#include <iomanip>

std::vector<ll> RSAworker::encode(std::vector<ll> data) {
    std::vector<ll> encoded_data;
    for (size_t i = 0; i < data.size(); i++) {
        encoded_data.push_back(pow_mod(data[i], d, n));
    }
    return encoded_data;
}

std::vector<ll> RSAworker::decode(std::vector<ll> data) {
    std::vector<ll> decoded_data;
    for (size_t i = 0; i < data.size(); i++) {
        decoded_data.push_back(pow_mod(data[i], c, n));
    }
    return decoded_data;
}

void RSAworker::configure() {
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

void RSAworker::gen_keys() {
    d = gen_random();
    while (d < 2 || d >= phi || gcd(d, phi) != 1) {
        d = gen_random();
    }

    std::vector<ll> res = algorithm_Euclid(phi, d);
    c = res[2];
    c = (c % phi + phi) % phi;
}

ll RSAworker::Hash(std::vector<ll> data) {
    ll hash = 0;
    for (size_t i = 0; i < data.size(); i++) {
        hash = data[i] ^ hash;
    }
    return hash;
}

ll RSAworker::sign(std::vector<ll> data) {
    ll h = Hash(data);
    if (h >= n) {
        std::cout << "Hash >= n" << std::endl;
        throw std::runtime_error("Hash >= n");
    }
    return pow_mod(h, c, n);
}

void RSAworker::verify(std::vector<ll> data, ll s) {
    ll h = Hash(data);
    ll e = pow_mod(s, d, n);
    if (e == h) {
        std::cout << "OK!" << std::endl;
    } else {
        std::cout << "Bad!" << std::endl;
    }
}