#include "MyDemo/math_api.h"
#include "math_utils.h"

namespace MyDemo {
namespace Math {

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

bool is_negative(int num) {
    return num < 0;
}

} // namespace Math
} // namespace MyDemo