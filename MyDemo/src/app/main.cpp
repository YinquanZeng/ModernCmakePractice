#include "MyDemo/math_api.h"
#include <iostream>

int main() {
    // std::cout << "MyDemo v" << PROJECT_VERSION << std::endl;
    std::cout << "3 + 5 = " << MyDemo::Math::add(3, 5) << std::endl;
    std::cout << "4 * 6 = " << MyDemo::Math::multiply(4, 6) << std::endl;
    return 0;
}
