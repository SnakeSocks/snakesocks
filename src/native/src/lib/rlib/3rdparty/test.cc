#include "prettyprint.hpp"
#include <rlib/stdio.hpp>
#include <list>
using namespace rlib;

int main() {
    std::list ls {1,3,2};
    _3rdparty::std::operator<<(std::cout, ls);
}
