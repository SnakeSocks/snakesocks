#include <rlib/log.hpp> //log_level_t
namespace rlib {
    namespace impl {
        bool enable_endl_flush = true;
        int max_predefined_log_level = (int)log_level_t::DEBUG;
    }
}
