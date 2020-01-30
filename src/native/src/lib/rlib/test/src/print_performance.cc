#include <rlib/functional.hpp>
#include <rlib/stdio.hpp>
#include <iostream>
using namespace rlib;
#include <cstdio>

#define test_str "This is some test string."
#define test_times 1000000

int main() {
    rlib::sync_with_stdio(false);
    println(std::cerr, timeof(repeat(test_times, []{
                   print(test_str "\n");
                   })));
    std::cerr << (timeof(repeat(test_times, []{
                   std::cout << test_str << "\n";
                   }))) << std::endl;
    std::cerr << (timeof(repeat(test_times, []{
                   std::printf(test_str);
                   std::printf("\n");
                   }))) << std::endl;
}
