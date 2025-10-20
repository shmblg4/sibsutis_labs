#include "cryptography.hpp"
#include <iostream>

int main() {
    try {
        Fileworker fileworker;
        std::string filename;
        std::cin >> filename;

        std::cout << "Читаем файл..." << std::endl;
        std::vector<ll> data = fileworker.read(filename);
        std::cout << "Прочитано байт: " << data.size() << std::endl;

        RSAworker worker;

        std::cout << "Вычисляем подпись..." << std::endl;
        ll s = worker.sign(data);
        std::cout << "Подпись: " << s << std::endl;

        std::cout << "Верифицируем подпись..." << std::endl;
        worker.verify(data, s);
        

    } catch (const std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}