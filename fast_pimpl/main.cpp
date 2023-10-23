#include <iostream>

#include "value.hpp"

int main() {

    thirdparty::super_havy::Value test, test2;

    test = std::move(test2);

    std::cout << test.Size() << "\n";

    std::cout << "exit\n";
    return 0;
}