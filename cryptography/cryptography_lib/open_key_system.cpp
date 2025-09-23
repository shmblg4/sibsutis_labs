#include "cryptography.hpp"

void disp_system_paramters(int p, int q, int g, User &user1, User &user2,
                           std::vector<int> &public_keys);
int find_g(int p, int q);

/** @brief Система открытого ключа
 *
 * @param user1 Пользователь 1
 * @param user2 Пользователь 2
 * В режиме работы 0, функция генерирует закрытые и открытые ключи
 * пользователей, в режиме 1 функция запрашивает пользователем закрытые ключи и
 * параметры p и g. Cимулируется обмен открытыми ключами и вычисление публичных
 * ключей
 * @return std::vector<int> Публичные ключи
 * Первый элемент вектора - ключ пользователя 1, второй - ключ пользователя 2
 */
std::vector<int> open_key_system(User &user1, User &user2) {
    std::random_device rd;
    std::mt19937 gen(rd());
    int mode = 0;
    std::cout << "\e[34m======================= Система открытого ключа "
                 "=======================\e[m"
              << std::endl;
    std::cout << "Режимы работы:\n"
              << "0. Генерация параметров\n"
              << "1. Ввод параметров\n";
    std::cout << "Выберите режим работы: ";
    std::cin >> mode;
    int q = gen() % 10000;
    while (ferm_test(q) != 1) {
        q = gen() % 10000;
    }
    int p, g;
    switch (mode) {
    case 1:
        std::cout << "Введите X1, X2, P и G: ";
        std::cin >> user1.secret_key >> user2.secret_key >> p >> g;
        break;
    case 0:
        p = 2 * q + 1;
        g = find_g(p, q);
        user1.secret_key = gen() % p;
        user2.secret_key = gen() % p;
        break;
    default:
        std::cout << "Некорректный режим работы" << std::endl;
        return {0, 0};
    }
    user1.open_key = pow_mod(g, user1.secret_key, p);
    user2.open_key = pow_mod(g, user2.secret_key, p);

    std::vector<int> public_keys = {0, 0};
    public_keys[0] = pow_mod(user2.open_key, user1.secret_key, p);
    public_keys[1] = pow_mod(user1.open_key, user2.secret_key, p);
    disp_system_paramters(p, q, g, user1, user2, public_keys);
    return public_keys;
}

int find_g(int p, int q) {
    std::random_device rd;
    std::mt19937 gen(rd());
    while (true) {
        int g = gen() % (p - 2) + 1;
        if (pow_mod(g, q, p) != 1) {
            return g;
        }
    }
}

void disp_system_paramters(int p, int q, int g, User &user1, User &user2,
                           std::vector<int> &public_keys) {
    std::cout << "p = " << p << ", q = " << q << ", g = " << g << std::endl;
    std::cout << "user1 secret key = " << user1.secret_key << std::endl;
    std::cout << "user1 open key = " << user1.open_key << std::endl;
    std::cout << "user2 secret key = " << user2.secret_key << std::endl;
    std::cout << "user2 open key = " << user2.open_key << std::endl;
    std::cout << "user1 public key = " << public_keys[0] << std::endl;
    std::cout << "user2 public key = " << public_keys[1] << std::endl;
}