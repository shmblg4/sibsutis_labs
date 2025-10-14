#include "cryptography.hpp"

ll ferm_test(ll n) {
    if (n < 2)
        return 0;
    return pow_mod(2, n - 1, n) == 1;
}

ll gen_prime() {
    std::random_device rd;
    std::mt19937 gen(rd());
    ll p = (gen() % 120) + 5; // Генерируем числа от 5 до 124
    while (!ferm_test(p)) {
        p = (gen() % 120) + 5;
    }
    return p;
}

ll gen_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    return (gen() % 1000) + 2; // Генерируем числа от 2 до 1001
}

ll gcd(ll a, ll b) {
    while (b != 0) {
        ll t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int Euler_algorithm(int n) {
    if (!ferm_test(n)) {
        return 0;
    }
    int result = 0;
    for (int i = 1; i < n; i++) {
        if (gcd(i, n) == 1) {
            result++;
        }
    }
    return result;
}

ll pow_mod(ll base, ll exp, ll mod) {
    if (mod <= 0)
        return 0;
    ll result = 1 % mod;
    ll cur = base % mod;
    while (exp > 0) {
        if (exp & 1)
            result = (result * cur) % mod;
        cur = (cur * cur) % mod;
        exp >>= 1;
    }
    return result;
}

std::vector<ll> algorithm_Euclid(ll a, ll b) {
    bool swapped = false;
    if (a < b) {
        std::swap(a, b);
        swapped = true;
    }
    std::vector<ll> U = {a, 1, 0};
    std::vector<ll> V = {b, 0, 1};
    ll q = 0;
    std::vector<ll> T(3);
    while (V[0] != 0) {
        q = U[0] / V[0];
        T = {U[0] % V[0], U[1] - q * V[1], U[2] - q * V[2]};
        U = V;
        V = T;
    }
    if (swapped)
        return {U[0], U[2], U[1]};
    else
        return U;
}