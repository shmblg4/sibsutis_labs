#include "cryptography.hpp"
#include <iostream>

int main() {
    try {
        RSAcoder coder;
        Fileworker fileworker;
        std::string filename;
        std::cin >> filename;

        std::cout << "Читаем файл..." << std::endl;
        std::vector<ll> data = fileworker.read(filename);
        std::cout << "Прочитано байт: " << data.size() << std::endl;

        std::cout << "Кодируем данные..." << std::endl;
        std::vector<ll> encoded_data = coder.encode(data);
        fileworker.write(encoded_data, "encoded_" + filename);
        std::cout << "Закодированные данные сохранены в encoded_text.txt"
                  << std::endl;

        std::cout << "Декодируем данные..." << std::endl;
        std::vector<ll> decoded_data = coder.decode(encoded_data);
        fileworker.write(decoded_data, "decoded_" + filename);
        std::cout << "Декодированные данные сохранены в decoded_text.txt"
                  << std::endl;

        // Проверка корректности
        bool is_correct = true;
        if (data.size() != decoded_data.size()) {
            is_correct = false;
            std::cout << "Ошибка: размеры не совпадают!" << std::endl;
        } else {
            for (size_t i = 0; i < data.size(); i++) {
                if (data[i] != decoded_data[i]) {
                    is_correct = false;
                    std::cout << "Ошибка в позиции " << i << ": " << data[i]
                              << " != " << decoded_data[i] << std::endl;
                    break;
                }
            }
        }

        if (is_correct) {
            std::cout << "✓ Данные корректно закодированы и декодированы!"
                      << std::endl;
        } else {
            std::cout << "✗ Обнаружены ошибки в кодировании/декодировании!"
                      << std::endl;
        }

    } catch (const std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}