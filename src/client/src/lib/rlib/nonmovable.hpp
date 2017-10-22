#ifndef R_NONMOVEABLE_HPP
#define R_NONMOVEABLE_HPP

#include "noncopyable.hpp"
namespace rlib {
    namespace _nonmv_ {
        class nonmovable : private noncopyable
        {
        public:
            nonmovable() = default;
            ~nonmovable() = default;
            nonmovable(const nonmovable &&) = delete;
            nonmovable &operator=(const nonmovable &&) = delete;
        };
    }
    typedef _nonmv_::nonmovable nonmovable;
}

#endif