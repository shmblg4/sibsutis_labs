#include <cryptography.hpp>

int main() {
    FileWorker file;
    ShamirCoder coder = ShamirCoder();
    std::vector<byte_t> bytes = file.read("file.txt");
    bytes = coder.worker(bytes);
    file.write(bytes, "output.txt");
    return 0;
}