#ifndef R_PRINT_HPP
#define R_PRINT_HPP

#include <iostream>

namespace rlib {
	template<typename PrintFinalT>
	void print(PrintFinalT reqArg)
	{
		::std::cout << reqArg;
		return;
	}
	template<typename Required, typename... Optional>
	void print(Required reqArgs, Optional... optiArgs)
	{
		::std::cout << reqArgs << ' ';
		print(optiArgs ...);
		return;
	}
	template<typename... Optional>
	void println(Optional... optiArgs)
	{
		print(optiArgs ...);
		::std::cout << ::std::endl;
		return;
	}

	template<typename Iterable, typename Printable>
	void print_iter(Iterable arg, Printable spliter)
	{
        for(const auto & i : arg)
		    ::std::cout << i << spliter;
		return;
	}
	template<typename Iterable, typename Printable>
	void println_iter(Iterable arg, Printable spliter)
	{
		print_iter(arg, spliter);
		::std::cout << ::std::endl;
		return;
	}
    template<typename Iterable>
	void print_iter(Iterable arg)
	{
        for(const auto & i : arg)
		    ::std::cout << i << ' ';
		return;
	}
	template<typename Iterable>
	void println_iter(Iterable arg)
	{
		print_iter(arg);
		::std::cout << ::std::endl;
		return;
	}
}

#endif
