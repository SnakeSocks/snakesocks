/*
 *
 * string.hpp: string process utility.
 * Recolic Keghart <root@recolic.net>
 * MIT License
 *
 */

#ifndef R_STRING_HPP
#define R_STRING_HPP

#include <rlib/require/cxx14>
#include <rlib/class_decorator.hpp>
#include <rlib/sys/os.hpp>

#include <array>
#include <vector>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <type_traits>

// Intel C++ compiler has a pending bug for `thread_local inline` variable.
#if RLIB_COMPILER_ID == CC_ICC
#define RLIB_IMPL_SSTREAM_DISABLE_TLS
#endif

#if RLIB_COMPILER_IS_MINGW
#define RLIB_IMPL_SSTREAM_DISABLE_TLS
#endif

namespace rlib {
    // literals::_format, format_string, string::format
    namespace impl {
#ifndef RLIB_IMPL_SSTREAM_DISABLE_TLS
        inline std::stringstream &to_string_by_sstream_ss() {
            static thread_local std::stringstream instance;
            return instance;
        }
        inline std::stringstream &_format_string_helper_ss() {
            static thread_local std::stringstream instance;
            return instance;
        }
#endif
        template <typename VarT>
        std::string to_string_by_sstream(VarT &thing) {
#if defined(RLIB_IMPL_SSTREAM_DISABLE_TLS)
            // Fix intel C++ bug https://software.intel.com/en-us/forums/intel-c-compiler/topic/784136
            // Also fix mingw bug. But much slower!
            std::stringstream ss;
#else
            auto &ss = to_string_by_sstream_ss();
            ss.str(std::string());
#endif
            ss << thing;
            return ss.str();
        }

        template<typename... Args>
        std::string _format_string_helper(const std::string &fmt, Args... args) {
#if defined(RLIB_IMPL_SSTREAM_DISABLE_TLS)
            // Fix intel C++ bug https://software.intel.com/en-us/forums/intel-c-compiler/topic/784136
            std::stringstream ss;
#else
            auto &ss = _format_string_helper_ss(); // cached stringstream is much quicker.
            ss.str(std::string());
#endif
            size_t pos = 0, prev_pos = 0;
            std::array<std::string, sizeof...(args)> argsArr{to_string_by_sstream(args) ...};
            size_t current_used_arg = 0;
            bool discovered_escape_char = false;
            while((pos = fmt.find("{}", pos)) != std::string::npos) {
                if(pos != 0 && fmt[pos-1] == '\\') {
                    // Invalid hit.
                    discovered_escape_char = true;
                    pos += 2;
                }
                else {
                    std::string cutted_tmp_str = fmt.substr(prev_pos, pos - prev_pos);
                    if(discovered_escape_char) {
                        // hand-written string replace. Replace `\{}` to `{}`.
                        size_t pos = 0;
                        while((pos = cutted_tmp_str.find("\\{}", pos)) != std::string::npos) {
                            cutted_tmp_str.erase(pos, 1);
                            pos += 2;
                        }
                    }
                    if(current_used_arg >= sizeof...(args)) {
                        // not enough arguments.
                        throw std::out_of_range("format_string: not enough arguments for format string `" + fmt +
                            "`. Expecting at least " + std::to_string(current_used_arg+1) + ", got " + 
                            std::to_string(sizeof...(args)));
                    }
                    ss << cutted_tmp_str << argsArr.at(current_used_arg);
                    pos += 2;
                    prev_pos = pos;
                    ++current_used_arg;
                }
            }
            if(current_used_arg != sizeof...(args)) {
                // too many arguments.
                throw std::out_of_range("format_string: too many arguments for format string `" + fmt +
                    "`. Expecting " + std::to_string(current_used_arg) + ", got " + 
                    std::to_string(sizeof...(args)));
            }
            ss << fmt.substr(prev_pos);
            return ss.str();
        }
        template<typename... Args>
        inline std::string format_string(const std::string &fmt, Args... args) {
            return _format_string_helper(fmt, args...);
        }
        //template<>
        //inline std::string format_string<>(const std::string &fmt) {
        //    return fmt;
        //}
 
        /*
        template<class MetaFmtArr, typename... Args>
        constexpr std::string format_string_meta(Args... args) {
            return (args + ...);
        }*/
    }

    // format_string_c, string::cformat
    namespace impl {
        inline char *_format_string_c_helper(const char *fmt, ...)
        {
            int n;
            int size = std::strlen(fmt);
            char *p, *np;
            va_list ap;

            if ((p = (char *)malloc(size)) == NULL)
                throw std::runtime_error("malloc returns null.");

            while (1) {
                va_start(ap, fmt);
                n = vsnprintf(p, size, fmt, ap);
                va_end(ap);

                if (n < 0)
                    throw std::runtime_error("vsnprintf returns " + std::to_string(n));
                if (n < size)
                    return p;

                size = n + 1;

                if ((np = (char *)realloc (p, size)) == NULL) {
                    free(p);
                    throw std::runtime_error("make_message realloc failed.");
                } else {
                    p = np;
                }
            }
        }
        template<typename... Args>
        std::string format_string_c(const std::string &fmt, Args... args)
        {
            char *res = _format_string_c_helper(fmt.c_str(), args ...);
            std::string s = res;
            free(res);
            return s;
        }
    }

    class string : public std::string {
    public:
        using std::string::string;
        string() : std::string() {}
        string(const std::string &s) : std::string(s) {}
        string(std::string &&s) : std::string(std::forward<std::string>(s)) {}

    private:
        template <typename T> struct as_helper {};
        template <typename T>
        T as(as_helper<T>) const {
            if(empty()) return T();
            return T(*this);
        }
        const char *as(as_helper<const char *>) const {
            return this->c_str();
        }
        std::string as(as_helper<std::string>) const {
            return std::move(*this);
        }
        rlib::string as(as_helper<rlib::string>) const {
            return std::move(*this);
        }
        char as(as_helper<char>) const {
            if(size() > 1)
                throw std::invalid_argument("Can not convert rlib::string to char: size() > 1.");
            return size() == 0 ? '\0' : *cbegin();
        }
        // unsigned-char conflicts with uint8_t. I'll regard it as uint8_t. ("8".as<unsigned char> == 8)
        //unsigned char as(as_helper<unsigned char>) const {
        //    return static_cast<unsigned char>(as<char>());
        //}
        bool as(as_helper<bool>) const {
            if(*this == "true") {
                return true;
            }
            else if(*this == "false") {
                return false;
            }
            // Nothing is slower than throw(); Just test more cases...
            else if(*this == "1" || *this == "True" || *this == "TRUE") {
                return true;
            }
            else if(*this == "0" || *this == "False" || *this == "FALSE") {
                return false;
            }
            throw std::invalid_argument("Can not convert rlib::string to bool. Not matching any template.");
        }

#define RLIB_IMPL_GEN_AS_NUMERIC(type, std_conv) \
        type as(as_helper<type>) const { \
            if(empty()) return 0; \
            return std::std_conv(*this); \
        }

        RLIB_IMPL_GEN_AS_NUMERIC(int, stoi)
        RLIB_IMPL_GEN_AS_NUMERIC(long, stol)
        RLIB_IMPL_GEN_AS_NUMERIC(unsigned long, stoul)
        RLIB_IMPL_GEN_AS_NUMERIC(unsigned long long, stoull)
        RLIB_IMPL_GEN_AS_NUMERIC(long long, stoll)
        RLIB_IMPL_GEN_AS_NUMERIC(float, stof)
        RLIB_IMPL_GEN_AS_NUMERIC(double, stod)
        RLIB_IMPL_GEN_AS_NUMERIC(long double, stold)

#define RLIB_IMPL_GEN_AS_ALIAS(new_type, old_type) \
        new_type as(as_helper<new_type>) const { \
            return static_cast<new_type>(as<old_type>()); \
        }

        RLIB_IMPL_GEN_AS_ALIAS(unsigned int, unsigned long)
        RLIB_IMPL_GEN_AS_ALIAS(unsigned short, unsigned long)
        RLIB_IMPL_GEN_AS_ALIAS(uint8_t, unsigned long)

        RLIB_IMPL_GEN_AS_ALIAS(short, int)
        RLIB_IMPL_GEN_AS_ALIAS(int8_t, int)

    public:
        template <typename T>
        T as() const {
            return std::forward<T>(as(as_helper<T>()));
        }

        std::vector<string> split(const char &divider = ' ') const {
            const string &toSplit = *this;
            std::vector<string> buf;
            size_t curr = 0, prev = 0;
            while((curr = toSplit.find(divider, curr)) != std::string::npos) {
                buf.push_back(toSplit.substr(prev, curr - prev));
                ++curr; // skip divider
                prev = curr;
            }
            buf.push_back(toSplit.substr(prev));
            return buf;
        }
        std::vector<string> split(const std::string &divider) const {
            const string &toSplit = *this;
            std::vector<string> buf;
            size_t curr = 0, prev = 0;
            while((curr = toSplit.find(divider, curr)) != std::string::npos) {
                buf.push_back(toSplit.substr(prev, curr - prev));
                curr += divider.size(); // skip divider
                prev = curr;
            }
            buf.push_back(toSplit.substr(prev));
            return buf;
        }
        template <typename T>
        std::vector<T> split_as(const char &divider = ' ') const {
            const string &toSplit = *this;
            std::vector<T> buf;
            size_t curr = 0, prev = 0;
            while((curr = toSplit.find(divider, curr)) != std::string::npos) {
                buf.push_back(string(toSplit.substr(prev, curr - prev)).as<T>());
                ++curr; // skip divider
                prev = curr;
            }
            buf.push_back(string(toSplit.substr(prev)).as<T>());
            return std::move(buf);
        }
        template <typename T>
        std::vector<T> split_as(const std::string &divider) const {
            const string &toSplit = *this;
            std::vector<T> buf;
            size_t curr = 0, prev = 0;
            while((curr = toSplit.find(divider, curr)) != std::string::npos) {
                buf.push_back(string(toSplit.substr(prev, curr - prev)).as<T>());
                curr += divider.size(); // skip divider
                prev = curr;
            }
            buf.push_back(string(toSplit.substr(prev)).as<T>());
            return std::move(buf);
        }

        template <class ForwardIterable>
        string &join(const ForwardIterable &buffer) {
            join(buffer.cbegin(), buffer.cend());
            return *this;
        }
        template <class ForwardIterator>
        string &join(ForwardIterator begin, ForwardIterator end) {
            const string &toJoin = *this;
            std::string result;
            for(ForwardIterator iter = begin; iter != end; ++iter) {
                if(iter != begin)
                    result += toJoin;
                result += *iter;
            }
            return operator=(std::move(result));
        }

        string &strip() {
            strip(" \t\r\n");
            return *this;
        }
        template <typename CharOrStringOrView>
        string &strip(const CharOrStringOrView &stripped) {
            size_t len = size();
            size_t begin = find_first_not_of(stripped);

            if(begin == std::string::npos) {
                clear();
                return *this;
            }
            size_t end = find_last_not_of(stripped);

            erase(end + 1, len - end - 1);
            erase(0, begin);
            return *this;
        }

        string &replace(const std::string &from, const std::string &to) {
            size_t _;
            replace(from, to, _);
            return *this;
        }
        string &replace(const std::string &from, const std::string &to, size_t &out_times) {
            if(from.empty())
                return *this;
            size_t start_pos = 0;
            size_t times = 0;
            while((start_pos = find(from, start_pos)) != std::string::npos)
            {
                ++times;
                this->std::string::replace(start_pos, from.length(), to);
                start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
            }
            out_times = times;
            return *this;
        }
        string &replace_once(const std::string &from, const std::string &to) {
            bool _;
            replace_once(from, to, _);
            return *this;
        }
        string &replace_once(const std::string &from, const std::string &to, bool &out_replaced) {
            size_t start_pos = find(from);
            if(start_pos == std::string::npos) {
                out_replaced = false;
            }
            else {
                this->std::string::replace(start_pos, from.length(), to);
                out_replaced = true;
            }
            return *this;
        }

        template <typename... Args>
        string &format(Args... args) {
            return operator=(std::move(impl::format_string(*this, args ...)));
        }
        template <typename... Args>
        string &cformat(Args... args) {
            return operator=(std::move(impl::format_string_c(*this, args ...)));
        }
    };

    namespace impl {
        struct formatter {
            formatter(const std::string &fmt) : fmt(fmt) {}
            formatter(std::string &&fmt) : fmt(fmt) {}
            template <typename... Args>
            std::string operator ()(Args... args) {
                return std::move(rlib::impl::format_string(fmt, args ...));
            }

            std::string fmt;
        };
    }

    namespace literals {
        inline impl::formatter operator "" _format (const char *str, size_t) {
            return impl::formatter(str);
        }
        inline rlib::string operator "" _rs (const char *str, size_t len) {
            return rlib::string(str, len);
        }
    }
}

#endif
