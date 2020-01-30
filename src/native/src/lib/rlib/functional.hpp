#ifndef RLIB_FUNCTIONAL_HPP_
#define RLIB_FUNCTIONAL_HPP_

#include <rlib/require/cxx17>
#include <rlib/class_decorator.hpp>
#include <rlib/sys/os.hpp>

#include <type_traits>
#include <list>
#include <functional>
#include <chrono>

namespace rlib {
    namespace impl {
        template <typename Func, typename... Args>
        struct repeated_func {
            using return_type = typename std::invoke_result<Func, Args ...>::type;
            auto operator ()(size_t count, Func f, Args ... args) {
                for(size_t cter = 0; cter < count - 1; ++cter)
                    f(std::forward<Args>(args) ...);
                return f(std::forward<Args>(args) ...);
            }
        };
        template <typename Func, typename... Args>
        struct repeated_func_return_list {
            using return_type = typename std::invoke_result<Func, Args ...>::type;
            auto operator ()(size_t count, Func f, Args ... args) {
                std::list<return_type> ret;
                if(count == 0) return ret;
                for(size_t cter = 0; cter < count; ++cter)
                    ret.push_back(std::move(f(std::forward<Args>(args) ...)));
                return ret;
            }
        };

    }
}

namespace rlib {
    template <class Func, typename... Args>
    constexpr static inline double timeof(Func && f, Args && ... args)
    {
        auto begin = std::chrono::high_resolution_clock::now();
        f(std::forward<Args>(args) ...);
        auto end = std::chrono::high_resolution_clock::now();
        return ::std::chrono::duration<double>(end - begin).count(); 
    }

    template <class Func, typename... Args>
    constexpr static inline auto repeat(size_t count, Func && f, Args && ... args)
    {
        // Unnecessary asserts for debugging.
        using return_type = typename std::invoke_result<Func, Args ...>::type;
        using return_type2 = typename std::invoke_result<typename impl::repeated_func<Func, Args ...>, size_t, Func, Args ...>::type;
        using return_type3 = decltype(impl::repeated_func<Func, Args ...>()(count, f, args ...));
        static_assert(std::is_same<return_type, return_type2>::value);
        static_assert(std::is_same<return_type, return_type3>::value);

        if(count == 0)
            throw std::invalid_argument("Can not repeat for zero times.");

        return std::bind(impl::repeated_func<Func, Args ...>(), count, std::forward<Func>(f), std::forward<Args>(args) ...);
    }
    template <class Func, typename... Args>
    constexpr static inline auto repeat_and_return_list(size_t count, Func f, Args... args)
    {
        return std::bind(impl::repeated_func_return_list<Func, Args ...>(), count, std::forward<Func>(f), std::forward<Args>(args) ...);
    }
}

namespace std {
#if RLIB_CXX_STD >= 2017
    class execution;
#endif
}

// functools here.
#if 0 // not finished
#include <algorithm>
namespace rlib {
    template <Iterable buffer_t>
    class wrappedIterable : public buffer_t {
    public:
        using buffer_t::buffer_t;
        using buffer_type = buffer_t;
        using value_type = buffer_t::value_type;
        using this_type = wrappedIterable<buffer_t>;
        wrappedIterable(const buffer_t &b) : buffer_t(b) {}
        wrappedIterable(buffer_t &&b) : buffer_t(std::forward<buffer_t>(b)) {}

#if RLIB_CXX_STD >= 2017
        // std::foreach wrapper
        this_type &map(std::execution &&policy, std::function<value_type(const value_type &)> mapper_func) {
            std::for_each(std::forward<std::execution>(policy), begin, end, [&mapper_func](value_type &v){v = mapper_func(v);});
        }
        this_type &map(std::execution &&policy, std::function<void(value_type &)> mapper_func) {
            std::for_each(std::forward<std::execution>(policy), begin, end, mapper_func);
        }


#endif
        // std::foreach wrapper
        this_type &map(std::function<value_type(const value_type &)> mapper_func) {
            std::for_each(begin, end, [&mapper_func](value_type &v){v = mapper_func(v);});
            return *this;
        }
        this_type &map(std::function<void(value_type &)> mapper_func) {
            std::for_each(begin, end, mapper_func);
            return *this;
        }

        this_type &filter(std::function<bool(const value_type &)> filter_func) {
            std::remove_if(begin, end, [&filter_func](const value_type &v) -> bool {return !filter_func(v);});
            return *this;
        }

        this_type &flat_map(std::function<buffer_type<value_type>(const value_type &)>) {

        }

    };
}
#endif

#endif
