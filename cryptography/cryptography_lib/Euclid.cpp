#include "cryptography.hpp"

void Euclid()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    int mode = 0;
    std::cout << "\e[34m======================= Метод Евклида "
                 "=======================\e[m"
              << std::endl;
    std::cout << "Режимы работы:\n"
              << "1. Ввод a и b с клавиатуры\n"
              << "2. Генерация a и b\n"
              << "3. Генерация a и b по правилу Ферма\n"
              << "Выберите режим работы: ";
    std::cin >> mode;
    int a, b;
    switch (mode)
    {
    case 1:
        std::cin >> a >> b;
        disp_Euclid_result(algorithm_Euclid(a, b), a, b);
        break;
    case 2:
        a = gen() % 100;
        b = gen() % a;
        std::cout << "a = " << a << ", b = " << b << std::endl;
        disp_Euclid_result(algorithm_Euclid(a, b), a, b);
        break;
    case 3:
        a = gen() % 100;
        while (!ferm_test(a))
            a = gen() % 100;
        b = gen() % a;
        while (!ferm_test(b))
            b = gen() % a;
        std::cout << "a = " << a << ", b = " << b << std::endl;
        disp_Euclid_result(algorithm_Euclid(a, b), a, b);
        break;
    default:
        std::cout << "Incorrect mode" << std::endl;
        break;
    }
}

std::vector<int> algorithm_Euclid(int a, int b)
{
    if (a < b)
    {
        throw std::invalid_argument("a должен быть больше b");
    }
    std::vector<int> U = {a, 1, 0};
    std::vector<int> V = {b, 0, 1};
    int q = 0;
    std::vector<int> T(3);
    while (V[0] != 0)
    {
        q = U[0] / V[0];
        T = {U[0] % V[0], U[1] - q * V[1], U[2] - q * V[2]};
        U = V;
        V = T;
    }
    return U;
}

void disp_Euclid_result(std::vector<int> result, int a, int b)
{
    std::cout << "gcd(" << a << ", " << b << ") = " << result[0] << std::endl;
    std::cout << "x = " << result[1] << ", y = " << result[2] << std::endl;
}