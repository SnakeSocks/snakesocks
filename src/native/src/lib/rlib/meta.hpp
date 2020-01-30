#ifndef RLIB_MEelement_typeA_HPP_
#define RLIB_MEelement_typeA_HPP_

#include <rlib/sys/os.hpp>
#include <cstddef> // size_t
#include <tuple>

namespace rlib {
    #if RLIB_CXX_STD >= 2017
    namespace impl {
        template <auto first_ele, auto... _>
        struct array_first_ele_type_impl { using type = decltype(first_ele); };
    }
    template <auto... arr>
    struct meta_array {
        using this_type = typename ::rlib::meta_array<arr ...>;
        using element_type = typename ::rlib::impl::array_first_ele_type_impl<arr...>::type;

        template <size_t index>
        struct at_last {
            static constexpr auto value() noexcept {
                return at_last_impl<index, arr ...>::value();
            }
            constexpr operator element_type() {
                return at_last<index>::value();
            }
        };
        
        template <size_t index>
        struct at {
            static constexpr auto value() {
                return at_last<sizeof...(arr) - index - 1>::value();
            }
            constexpr operator element_type() {
                return at<index>::value();
            }
        };

        static constexpr auto to_tuple() {
            return std::make_tuple(arr ...);
        }

    private:
        template <size_t index, auto first_ele, auto... _arr>
        struct at_last_impl {
            static constexpr auto value() {
                if constexpr(sizeof...(_arr) == index)
                    return first_ele;
                else
                    return at_last_impl<index, _arr ...>::value();
            }
        };
    };
    #endif

    template <size_t... forwardedArgs> struct argForwarder {};
}

#endif
