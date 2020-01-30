#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <rlib/string.hpp>
#include <stdexcept>

using namespace rlib::literals;

TEST_CASE("literal _format", "[string_format]") {
    REQUIRE("fuck {} at {} a.m."_format("hust", 8) == "fuck hust at 8 a.m.");
    REQUIRE("test {}/{}"_rs .format("hi", 1.234) == "test hi/1.234");
    REQUIRE("\\{},\\\\\\{},\\abc,{}."_format("you") == "{},\\\\{},\\abc,you.");
    REQUIRE("{{}}, {, }{"_format("test") == "{test}, {, }{");
    REQUIRE("\\{ \\} \\}{ \\{{ \\{} {}"_format("{}") == "\\{ \\} \\}{ \\{{ {} {}");
    REQUIRE_THROWS_AS("\\n{}"_format(), std::out_of_range);
    REQUIRE_THROWS_AS("\\n{}{}"_format("{}"), std::out_of_range);
}

TEST_CASE("rlib::string as", "[string_as]") {
    REQUIRE("123"_rs .as<int>() == 123);
    REQUIRE("8"_rs .as<uint8_t>() == 8);
    REQUIRE("8"_rs .as<int8_t>() == 8);
    REQUIRE("8"_rs .as<char>() == '8');
    REQUIRE("8"_rs .as<unsigned char>() == 8); // not a bug.
    REQUIRE("hello"_rs .as<std::string>() == "hello");
    REQUIRE("hello"_rs .as<rlib::string>() == "hello");
    REQUIRE(std::string("hello"_rs .as<const char *>()) == "hello");
    REQUIRE("1"_rs .as<bool>() == true);
    REQUIRE("false"_rs .as<bool>() == false);
}

TEST_CASE("rlib::string others", "[string_op]") {
    rlib::string test_str = "|123||";
    REQUIRE(test_str.split('|')[1] == "123");
    REQUIRE(test_str.split('|')[3] == "");
    REQUIRE(test_str.split_as<int>('|')[1] == 123);
    REQUIRE(test_str.split_as<int>('|')[2] == 0);

    REQUIRE("\r fuck you \t \n"_rs .strip() == "fuck you");
    REQUIRE("hello world"_rs .strip("ld") == "hello wor");
    REQUIRE(""_rs .strip("") == "");

    REQUIRE("your mother suck fuck you"_rs.replace("you", "abc").strip("ac").split()[4] == "ab");
    REQUIRE("fuck you"_rs.replace("", "") == "fuck you");
    REQUIRE("123"_rs.replace("", "1") == "123");
    REQUIRE("hello worlo"_rs.replace("lo", "") == "hel wor");

    test_str = "hello world \n abc def some   random  ";
    auto test_str2 = test_str;
    REQUIRE(" "_rs.join(test_str.split(' ')) == test_str2);
}






