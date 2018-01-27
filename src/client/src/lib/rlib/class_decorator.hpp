#ifndef RLIB_CLASS_DECO_HPP_
#define RLIB_CLASS_DECO_HPP_

#include <rlib/require/cxx11>

namespace rlib {
    namespace _noncp_ {
        class noncopyable
        {
        public:
            noncopyable() = default;
            ~noncopyable() = default;
            noncopyable(const noncopyable &) = delete;
            noncopyable &operator=(const noncopyable &) = delete;
        };
    }
    typedef _noncp_::noncopyable noncopyable;
}

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

namespace rlib {
    namespace _nonconstructible_ {
        class nonconstructible : private rlib::nonmovable
        {
        public:
            nonconstructible() = delete;
            ~nonconstructible() = delete;
        };
    }
    typedef _nonconstructible_::nonconstructible nonconstructible;
    typedef nonconstructible static_class;
}

#endif