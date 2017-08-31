#ifndef R_NONCOPYABLE_HPP
#define R_NONCOPYABLE_HPP

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

#endif