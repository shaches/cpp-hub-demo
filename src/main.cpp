// src/main.cpp
#include <iostream>
#include "cpp_hub/cli.hpp"

int main(int argc, char** argv) {
    try {
        return cpp_hub::cli::run(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
}

