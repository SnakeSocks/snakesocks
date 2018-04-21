#ifndef RLIB_MEelement_typeA_HPP_
#define RLIB_MEelement_typeA_HPP_

#include <rlib/require/cxx17>
#include <cstddef> // size_t

namespace rlib::impl {
    template <auto first_ele, auto... _>
    struct array_first_ele_type_impl { using type = decltype(first_ele); };
}

namespace rlib {
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
                return at_last<sizeof...(arr) - index - 1, arr ...>::value();
            }
            constexpr operator element_type() {
                return at<index>::value();
            }
        };

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
}

#endif
