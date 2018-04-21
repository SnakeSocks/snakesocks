#ifndef RLIB_TRAITS_HPP
#define RLIB_TRAITS_HPP

#include <type_traits>

namespace rlib{
    namespace impl {
        template<typename T>
        struct is_callable_helper {
        private:
            typedef char(&yes)[1];
            typedef char(&no)[2];
    
            struct Fallback { void operator()(); };
            struct Derived : T, Fallback { };
    
            template<typename U, U> struct Check;
    
            template<typename>
            static yes test(...);
    
            template<typename C>
            static no test(Check<void (Fallback::*)(), &C::operator()>*);
    
        public:
            static constexpr bool value = sizeof(test<Derived>(0)) == sizeof(yes);
        };
    } //impl
} //rlib

namespace rlib {
    template<typename T>
    struct is_callable {
        using _impl = typename std::conditional<std::is_class<T>::value, impl::is_callable_helper<T>, std::is_function<T>>::type;
        static constexpr bool value() noexcept {
            return _impl::value;
        }
        constexpr operator bool() noexcept {
            return is_callable<T>::value();
        }
    };
}

#endif
