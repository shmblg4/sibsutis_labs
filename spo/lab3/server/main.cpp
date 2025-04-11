#include "server.hpp"
#include <iostream>
#include <csignal>

volatile std::sig_atomic_t g_signal_received = 0;

void signal_handler(int sig) {
    g_signal_received = sig;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        dhcp::DHCPServer server;
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}