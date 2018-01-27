#ifndef R_STDIO_HPP
#define R_STDIO_HPP

#include <rlib/require/cxx11>
// Must link libr.a
#include <string>
#include <iostream>
#include <rlib/string/string.hpp>

namespace rlib {
    template<typename PrintFinalT>
    void print(PrintFinalT reqArg);
    template<typename Required, typename... Optional>
    void print(Required reqArgs, Optional... optiArgs);
    template<typename... Optional>
    void println(Optional... optiArgs);
    void println();

    template<typename Iterable, typename Printable>
    void print_iter(Iterable arg, Printable spliter);
    template<typename Iterable, typename Printable>
    void println_iter(Iterable arg, Printable spliter);
    template<typename Iterable>
    void print_iter(Iterable arg);
    template<typename Iterable>
    void println_iter(Iterable arg);

    template<typename... Args>
    size_t printf(const std::string &fmt, Args... args);
    template<typename... Args>
    size_t printfln(const std::string &fmt, Args... args);

    inline std::string scanln()
    {
        ::std::string line;
        ::std::getline(::std::cin, line);
        return std::move(line);
    }

// Implements.
    extern bool enable_endl_flush;
    template< class CharT, class Traits >
    std::basic_ostream<CharT, Traits>& endl(std::basic_ostream<CharT, Traits>& os) {
        os << '\n';
        if(enable_endl_flush)
            os.flush();
        return os;
    }

    template<typename PrintFinalT>
    void print(PrintFinalT reqArg)
    {
        ::std::cout << reqArg;
    }
    template<typename Required, typename... Optional>
    void print(Required reqArgs, Optional... optiArgs)
    {
        ::std::cout << reqArgs << ' ';
        print(optiArgs ...);
    }
    template<typename... Optional>
    void println(Optional... optiArgs)
    {
        print(optiArgs ...);
        println();
    }
    inline void println()
    {
        ::std::cout << ::rlib::endl;
    }

    template<typename Iterable, typename Printable>
    void print_iter(Iterable arg, Printable spliter)
    {
        for(const auto & i : arg)
            ::std::cout << i << spliter;
    }
    template<typename Iterable, typename Printable>
    void println_iter(Iterable arg, Printable spliter)
    {
        print_iter(arg, spliter);
        ::std::cout << ::rlib::endl;
    }
    template<typename Iterable>
    void print_iter(Iterable arg)
    {
        for(const auto & i : arg)
            ::std::cout << i << ' ';
    }
    template<typename Iterable>
    void println_iter(Iterable arg)
    {
        print_iter(arg);
        ::std::cout << ::rlib::endl;
    }

    template<typename... Args>
    size_t printf(const std::string &fmt, Args... args)
    {
        std::string to_print = format_string(fmt, args...); 
        ::std::cout << to_print;
        return to_print.size();
    }
    template<typename... Args>
    size_t printfln(const std::string &fmt, Args... args)
    {
        size_t len = ::rlib::printf(fmt, args...);
        ::std::cout << ::rlib::endl;
        return len + 1;
    }
}


#endif
