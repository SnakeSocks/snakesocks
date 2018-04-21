/*
 *
 * stdio wrapper for modern c++: python like print/println/printf/printfln
 *                               print_iter println_iter
 * MIT License
 *
 */

#ifndef R_STDIO_HPP
#define R_STDIO_HPP

#include <rlib/require/cxx11> // Use fold expression if cxx17 is available.
#include <string>
#include <iostream>
#include <rlib/string.hpp> // format_string
#include <unistd.h> // STDOUT_FILENO

#if RLIB_OS_ID == OS_WINDOWS
#define RLIB_IMPL_ENDLINE "\r\n"
#elif RLIB_OS_ID == OS_MACOS
#define RLIB_IMPL_ENDLINE "\r"
#else
#define RLIB_IMPL_ENDLINE "\n"
#endif

namespace rlib {
// print to custom stream
    template <typename PrintFinalT>
    void print(std::ostream &os, PrintFinalT reqArg);
    template <typename Required, typename... Optional>
    void print(std::ostream &os, Required reqArgs, Optional... optiArgs);
    template <typename... Optional>
    void println(std::ostream &os, Optional... optiArgs);
    template <>
    void println(std::ostream &os);

    template <typename Iterable, typename Printable>
    void print_iter(std::ostream &os, Iterable arg, Printable spliter);
    template <typename Iterable, typename Printable>
    void println_iter(std::ostream &os, Iterable arg, Printable spliter);
    template <typename Iterable>
    void print_iter(std::ostream &os, Iterable arg);
    template <typename Iterable>
    void println_iter(std::ostream &os, Iterable arg);

    template <typename... Args>
    size_t printf(std::ostream &os, const std::string &fmt, Args... args);
    template <typename... Args>
    size_t printfln(std::ostream &os, const std::string &fmt, Args... args);

    inline rlib::string scanln(std::istream &is = std::cin, char delimiter = '\n') noexcept {
        std::string line;
        std::getline(is, line, delimiter);
        return std::move(line);
    }

// print to stdout
    template <typename PrintFinalT>
    void print(PrintFinalT reqArg);
    template <typename Required, typename... Optional>
    void print(Required reqArgs, Optional... optiArgs);
    template <typename... Optional>
    void println(Optional... optiArgs);
    template <>
    void println();

    template <typename Iterable, typename Printable>
    void print_iter(Iterable arg, Printable spliter);
    template <typename Iterable, typename Printable>
    void println_iter(Iterable arg, Printable spliter);
    template <typename Iterable>
    void print_iter(Iterable arg);
    template <typename Iterable>
    void println_iter(Iterable arg);

    template <typename... Args>
    size_t printf(const std::string &fmt, Args... args);
    template <typename... Args>
    size_t printfln(const std::string &fmt, Args... args);

    namespace impl {extern bool enable_endl_flush;}

    inline bool sync_with_stdio(bool sync = true) noexcept {
        return std::ios::sync_with_stdio(sync);
    }
    inline bool enable_endl_flush(bool enable = true) noexcept {
        return impl::enable_endl_flush = enable;
    }

// Implements.
    template < class CharT, class Traits >
    inline std::basic_ostream<CharT, Traits>& endl(std::basic_ostream<CharT, Traits>& os) {
        os << RLIB_IMPL_ENDLINE;
        if(impl::enable_endl_flush)
            os.flush();
        return os;
    }

    template <typename PrintFinalT>
    void print(PrintFinalT reqArg)
    {
        std::cout << reqArg;
    }
    template <typename Required, typename... Optional>
    void print(Required reqArgs, Optional... optiArgs)
    {
        std::cout << reqArgs << ' ';
        print(optiArgs ...);
    }
    template <typename... Optional>
    void println(Optional... optiArgs)
    {
        print(optiArgs ...);
        println();
    }
    template <> 
    inline void println()
    {
        std::cout << rlib::endl;
    }

    template <typename Iterable, typename Printable>
    void print_iter(Iterable arg, Printable spliter)
    {
        for(const auto & i : arg)
            std::cout << i << spliter;
    }
    template <typename Iterable, typename Printable>
    void println_iter(Iterable arg, Printable spliter)
    {
        print_iter(arg, spliter);
        std::cout << rlib::endl;
    }
    template <typename Iterable>
    void print_iter(Iterable arg)
    {
        for(const auto & i : arg)
            std::cout << i << ' ';
    }
    template <typename Iterable>
    void println_iter(Iterable arg)
    {
        print_iter(arg);
        std::cout << rlib::endl;
    }

    template <typename... Args>
    size_t printf(const std::string &fmt, Args... args)
    {
        std::string to_print = impl::format_string(fmt, args...); 
        std::cout << to_print;
        return to_print.size();
    }
    template <typename... Args>
    size_t printfln(const std::string &fmt, Args... args)
    {
        size_t len = rlib::printf(fmt, args...);
        std::cout << rlib::endl;
        return len + 1;
    }

// With custom os
    template <typename PrintFinalT>
    void print(std::ostream &os, PrintFinalT reqArg)
    {
        os << reqArg;
    }
    template <typename Required, typename... Optional>
    void print(std::ostream &os, Required reqArgs, Optional... optiArgs)
    {
        os << reqArgs << ' ';
        print(os, optiArgs ...);
    }
    template <typename... Optional>
    void println(std::ostream &os, Optional... optiArgs)
    {
        print(os, optiArgs ...);
        println();
    }
    template <> 
    inline void println(std::ostream &os)
    {
        os << rlib::endl;
    }

    template <typename Iterable, typename Printable>
    void print_iter(std::ostream &os, Iterable arg, Printable spliter)
    {
        for(const auto & i : arg)
            os << i << spliter;
    }
    template <typename Iterable, typename Printable>
    void println_iter(std::ostream &os, Iterable arg, Printable spliter)
    {
        print_iter(os, arg, spliter);
        os << rlib::endl;
    }
    template <typename Iterable>
    void print_iter(std::ostream &os, Iterable arg)
    {
        for(const auto & i : arg)
            os << i << ' ';
    }
    template <typename Iterable>
    void println_iter(std::ostream &os, Iterable arg)
    {
        print_iter(os, arg);
        os << rlib::endl;
    }

    template <typename... Args>
    size_t printf(std::ostream &os, const std::string &fmt, Args... args)
    {
        std::string to_print = format_string(fmt, args...); 
        os << to_print;
        return to_print.size();
    }
    template <typename... Args>
    size_t printfln(std::ostream &os, const std::string &fmt, Args... args)
    {
        size_t len = rlib::printf(fmt, args...);
        os << rlib::endl;
        return len + 1;
    }
}


#endif
