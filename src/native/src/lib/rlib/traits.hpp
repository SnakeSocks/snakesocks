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
    
            static constexpr bool value = sizeof(test<Derived>(0)) == sizeof(yes);
        public:
            static constexpr bool real_value = std::conditional<std::is_class<T>::value, impl::is_callable_helper<T>, std::is_function<T>>::type::value;
        };
    } //impl
} //rlib

namespace rlib {
    template<typename T>
    struct is_callable : public std::bool_constant<impl::is_callable_helper<T>::real_value> {
    };
}

#endif
