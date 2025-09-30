#include "cryptography.hpp"

int ferm_test(int n) {
    return pow_mod(2, n - 1, n) == 1;
}

int gen_prime() {
    std::random_device rd;
    std::mt19937 gen(rd());
    int p = gen() % 10000;
    while (!ferm_test(p)) {
        p = gen() % 10000;
    }
    return p;
}

int Euler_algorithm(int n) {
    if (!ferm_test(n)) {
        return 0;
    }
    int result = 0;
    for (int i = 1; i < n; i++) {
        if (ferm_test(i)) {
            result++;
        }
    }
    return result;
}

/*
 * Возведение в степень по модулю
 * @param base основание
 * @param exp степень
 * @param mod модуль
 * @return результат
 */
int pow_mod(int base, int exp, int mod) {
    int result = 1;
    int s = base;
    int t = int(floor(log2(exp)));
    for (int i = 0; i < t + 1; i++) {
        if ((exp >> i) & 1) {
            result = (result * s) % mod;
        }
        s = (s * s) % mod;
    }
    return result;
}