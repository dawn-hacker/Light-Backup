#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

std::string executeCommand(const char* command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
    const char* target = "127.0.0.1"; // Replace with your target IP or hostname
    std::string command = "nmap -sC -sV ";
    command += target;
    try {
        std::string output = executeCommand(command.c_str());
        std::cout << "Nmap Output:\n" << output << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}

