// Compiled with /fp:fast on MSVC (see CMakeLists.txt and dllmath.h).
#include "core/dllmath.h"

#include <cmath>

namespace avas {

void dll_sincos(double a, double& s, double& c) {
    s = std::sin(a);
    c = std::cos(a);
}

}  // namespace avas
