#include <rlib/require/cxx11>
#include <streambuf>
#include <iostream>
#include <rlib/sys/os.hpp>

namespace rlib {
    namespace impl {
        class NullStreamBuf : public std::streambuf
        {
        public:
            int overflow(int c) { return c; }
        };

        inline NullStreamBuf & null_streambuf() {
            static NullStreamBuf instance;
            return instance;
        }
    }
    
    inline std::ostream &null_stream() {
        static std::ostream instance(impl::null_streambuf());
        return instance;
    }
}
