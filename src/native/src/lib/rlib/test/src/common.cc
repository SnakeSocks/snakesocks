#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <rlib/stdio.hpp>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <chrono>

void test_f(int);
class test_c {
public:
    auto operator()(int a) {
        return a;
    }
};

#if RLIB_CXX_STD >= 2017
#include <rlib/functional.hpp>
#include <rlib/traits.hpp>
TEST_CASE("functional") {
    std::stringstream test_ss;
    auto test_func = [&](int i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        rlib::println(test_ss, "i", i);
        return i;
    };

    auto time_cost = rlib::timeof(rlib::repeat(10, test_func, 1));
    REQUIRE(time_cost > 0.7);
    REQUIRE_THROWS_AS(rlib::timeof(rlib::repeat(0, test_func, 1)), std::invalid_argument);

    std::string answer;
    for(auto cter = 0; cter < 10; ++cter) {
        answer += "i 1" RLIB_IMPL_ENDLINE;
    }
    REQUIRE(test_ss.str() == answer);

    auto result = rlib::repeat(2, test_func, 1)();
    REQUIRE(result == 1);
    auto result2 = rlib::repeat_and_return_list(2, test_func, 2)();
    REQUIRE(result2.size() == 2);
    REQUIRE(*result2.begin() == 2);
}

TEST_CASE("traits") {
    auto lmbda = []()->bool{return true;};
    REQUIRE(rlib::is_callable<test_c>());
    REQUIRE(rlib::is_callable<decltype(test_f)>());
    REQUIRE(rlib::is_callable<void()>());
    REQUIRE(rlib::is_callable<decltype(lmbda)>());
    REQUIRE(rlib::is_callable<std::function<void(int)>>());
    REQUIRE_FALSE(rlib::is_callable<int>());
    REQUIRE_FALSE(rlib::is_callable<std::ostream>());
}
#endif // RLIB_CXX_STD > 2017

struct rlib_test_printable {
    int d = 1;
    friend std::ostream& operator<< (std::ostream& stream, const rlib_test_printable & p) {
        stream << p.d;
        return stream;
    }
};
TEST_CASE("stdio.hpp") {
    std::stringstream test_ss;
    rlib::print(test_ss, '>');
    rlib::println(test_ss, "a", 'b', 123, 0.25, rlib_test_printable{2});
    REQUIRE(test_ss.str() == ">a b 123 0.25 2" RLIB_IMPL_ENDLINE);

    rlib::printf(test_ss, "{}{}", 1, "");
    rlib::printfln(test_ss, "hello, {}.", "godaddy");
    REQUIRE(rlib::scanln(test_ss) == ">a b 123 0.25 2");
    REQUIRE(rlib::scanln(test_ss) == "1hello, godaddy.");
}


