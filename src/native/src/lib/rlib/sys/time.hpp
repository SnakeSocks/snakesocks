#ifndef RLIB_TIME_HPP_
#define RLIB_TIME_HPP_

#include <chrono>
#include <ctime>
#include <iomanip>
namespace rlib {
    static inline std::string get_current_time_str() noexcept {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(24));
        static char mbstr[128];
        if (std::strftime(mbstr, sizeof(mbstr), "%c", std::localtime(&now_c))) {
            return mbstr;
        }
        throw std::overflow_error("on get_current_time: mbstr buffer is too small.");
    }
}
#endif
