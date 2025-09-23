#include "cryptography.hpp"

std::vector<int> algorithm_small_big_step(int a, int y, int p) {
    int m = static_cast<int>(std::ceil(std::sqrt(p)));
    int k = m;
    std::vector<Step> U;
    std::vector<Step> V;
    std::vector<int> result;
    U.reserve(m);
    V.reserve(k);
    for (int i = 0; i < m; i++) {
        U.push_back({i, static_cast<int>(std::pow(a, i)) * y % p});
    }
    for (int i = 1; i < k + 1; i++) {
        V.push_back({i, static_cast<int>(std::pow(a, m * i)) % p});
    }
    std::sort(U.begin(), U.end(),
              [](const Step &a, const Step &b) { return a.val < b.val; });
    std::sort(V.begin(), V.end(),
              [](const Step &a, const Step &b) { return a.val < b.val; });

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < m; j++) {
            if (U[i].val == V[j].val) {
                result.push_back(V[j].index * m - U[i].index);
            }
        }
    }
    return result;
}