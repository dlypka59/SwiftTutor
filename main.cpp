// SWIFT Interview Tutor - C++23
// Uses std::span, std::ranges, std::expected, std::format
// Compatible with Visual Studio 2022/2025/2026 (set language standard to C++23 /std:c++latest)

#include "Tutor.hpp"

#include <iostream>
#include <format>

int main() {
    std::cout << "SWIFT Banking Message Tutor  -  C++23 edition\n";
    std::cout << "Focused on MT103 / MT202 knowledge for interviews\n\n";

    try {
        swift::tutor::run_menu();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "\nGood luck with the interview!\n";
    return 0;
}
