#include "cryptography.hpp"

void small_big_step() {
    std::random_device rd;
    std::mt19937 gen(rd());
    int mode = 0;
    std::cout
        << "\e[34m======================= Метод « Шаг младенца, шаг великана » "
           "=======================\e[m"
        << std::endl;
    std::cout << "Режимы работы:\n"
              << "1. Ввод a, y, p с клавиатуры\n"
              << "2. Генерация параметров\n"
              << "Выберите режим работы: ";
    std::cin >> mode;
    int a, y, p;
    std::vector<int> res;
    int i = 1;
    switch (mode) {
    case 1:
        std::cin >> a >> y >> p;
        res = algorithm_small_big_step(a, y, p);
        std::cout << "a = " << a << ", y = " << y << ", p = " << p << std::endl;
        for (auto elem : res) {
            std::cout << "x" << i++ << " = " << elem << '\n';
        }
        std::cout << std::endl;
        break;
    case 2:
        a = gen() % 100;
        p = gen() % 100;
        while (ferm_test(p) != 1) {
            p = gen() % 100;
        }
        y = gen() % p;
        std::cout << "a = " << a << ", y = " << y << ", p = " << p << std::endl;
        res = algorithm_small_big_step(a, y, p);
        for (auto elem : res) {
            std::cout << "x" << i++ << " = " << elem << '\n';
        }
        std::cout << std::endl;
        break;
    default:
        std::cout << "Некорректный ввод" << std::endl;
        break;
    }
}