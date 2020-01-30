#include <rlib/meta.hpp>
#include <string>

int main() {
    static_assert(rlib::meta_array<33,34,35,36,37>::at<1>() == 34);
    static_assert(rlib::meta_array<'f','u','c','k'>::at<2>() == 'c');
    static_assert(std::get<1>(rlib::meta_array<'f','u','c','k'>::to_tuple()) == 'u');
}
