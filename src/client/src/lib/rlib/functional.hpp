#ifndef RLIB_FUNCTIONAL_HPP_
#define RLIB_FUNCTIONAL_HPP_

#include <rlib/require/cxx14>
#include <rlib/class_decorator.hpp>

#include <type_traits>
#include <list>
#include <functional>
#include <chrono>

namespace rlib {
    template <class operation_t, typename... args_t>
    static inline double timed_func(::std::function<operation_t> f, args_t... args)
    {
        auto begin = std::chrono::high_resolution_clock::now();
        f(args ...);
        auto end = std::chrono::high_resolution_clock::now();
        return ::std::chrono::duration<double>(end - begin).count(); 
    }

    template <class operation_t, typename... args_t>
    static inline typename ::std::result_of<operation_t(args_t ...)>::type repeat(size_t count, operation_t f, args_t... args)
    {
        for(size_t cter = 0; cter < count - 1; ++cter)
            f(args ...);
        return ::std::move(f(args ...));
    }
    template <class operation_t, typename... args_t>
    static inline ::std::list<typename ::std::result_of<operation_t(args_t ...)>::type> repeat_and_return_list(size_t count, operation_t f, args_t... args)
    {
        ::std::list<typename ::std::result_of<operation_t(args_t ...)>::type> ret;
        for(size_t cter = 0; cter < count; ++cter)
            ret.push_back(std::move(f(args ...)));
        return std::move(ret);
    }
}
#endif
