#include <iostream>
#include <rlib/terminal.hpp>
using namespace rlib::terminal;
int main()
{
	std::cout << color_t::red << font_t::bold << "hello world" << font_t::dark << "qaq..." << clear << "66666" << std::endl;
	std::cout << color_t::green << "Hi~" << font_t::underline << "Miaow" << std::endl;
	std::cout << "Continue!" << clear << std::endl;
	return 0;
}
