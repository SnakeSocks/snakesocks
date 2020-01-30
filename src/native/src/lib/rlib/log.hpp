#ifndef RLIB_LOG_HPP_
#define RLIB_LOG_HPP_ 1

#include <string>
#include <fstream>
#include <list>
#include <climits>
#include <rlib/sys/os.hpp>
#include <rlib/stdio.hpp>
#include <rlib/sys/time.hpp>
#include <rlib/class_decorator.hpp>

// currently disable this error-prone shit.
#define RLIB_IMPL_ENABLE_LOGGER_FROM_FD 0

#ifndef RLIB_IMPL_ENABLE_LOGGER_FROM_FD
#include <rlib/sys/fd.hpp>
#if RLIB_OS_ID != OS_UNKNOWN
#   if RLIB_COMPILER_ID == CC_GCC
#       include <ext/stdio_filebuf.h>
#       define RLIB_IMPL_ENABLE_LOGGER_FROM_FD 1
#   elif RLIB_COMPILER_ID == CC_MSVC
#       define RLIB_IMPL_ENABLE_LOGGER_FROM_FD 1
#   endif
#endif
#endif

#ifdef ERROR
#pragma message (": warning MSVC_Macro_pollution: You MUST NOT define the macro `ERROR`. I've undefined it here.")
#undef ERROR
#endif

namespace rlib {
    using namespace rlib::literals;

    // Allow extension.
    enum class log_level_t : int { FATAL = 1, ERROR, WARNING, INFO, VERBOSE, DEBUG };
    namespace impl {
        inline int &max_predefined_log_level() {
            static int instance = (int)log_level_t::DEBUG;
            return instance;
        }
    }
    /*
    How to update log_level_t:
        Extend `enum log_level_t ...`
        Modify libr.cc:`max_predefined_log_level ...`
        Add an RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND
        Append logger::predefined_log_level_name
    */

    class logger : rlib::noncopyable {
    public:
        logger() = delete;
        logger(std::ostream &stream) : pstream(&stream) {}
        logger(const std::string &file_name) : pstream(new std::ofstream(file_name, std::ios::out)), 
            must_delete_stream_as_ofstream(true) {
                if(!dynamic_cast<std::ofstream &>(*pstream))
                    throw std::runtime_error("Failed to open file {}."_format(file_name));
            }
        logger(logger &&another) : pstream(another.pstream), 
            custom_log_level_names(std::move(another.custom_log_level_names)),
            log_level(another.log_level),
            must_delete_stream_as_ofstream(another.must_delete_stream_as_ofstream),
            enable_flush(another.enable_flush)
            {another.must_delete_stream_as_ofstream = false;}
        ~logger() {
            if(must_delete_stream_as_ofstream)
                delete dynamic_cast<std::ofstream *>(pstream);
        }

        logger &operator=(logger &&another) {
            pstream = another.pstream;
            enable_flush = another.enable_flush;
            must_delete_stream_as_ofstream = another.must_delete_stream_as_ofstream;
            log_level = another.log_level;
            custom_log_level_names = std::move(another.custom_log_level_names);
            another.must_delete_stream_as_ofstream = false;
            return *this;
        }
        
#if RLIB_IMPL_ENABLE_LOGGER_FROM_FD == 1
#if RLIB_OS_ID != OS_UNKNOWN
        logger(fd file_descriptor_or_handle)
#   if RLIB_COMPILER_ID == CC_GCC
        : _gcc_filebuf(file_descriptor_or_handle, std::ios::out), _gcc_real_stream(&_gcc_filebuf), 
            stream(_gcc_real_stream) {}
#   elif RLIB_COMPILER_ID == CC_MSVC
        : _msvc_real_stream(::_fdopen(file_descriptor_or_handle, "w")), 
            stream(_msvc_real_stream) {}
#   endif
#endif
#endif

        void set_log_level(log_level_t max_level) {
            this->log_level = max_level;
        }
        void set_flush(bool enable_flush) noexcept {
            this->enable_flush = enable_flush;
        }
        template <typename ... Args>
        void log(log_level_t level, const std::string &info, Args ... extra_args) const {
            if(is_predefined_log_level(level) && level > this->log_level)
                return;
            (*pstream) << "[{}|{}] {}"_format(get_current_time_str(), log_level_name(level), impl::format_string(info, std::forward<Args>(extra_args) ...)) << RLIB_IMPL_ENDLINE;
            if(enable_flush)
                pstream->flush();
        }
        // Warning: this method is not thread-safe.
        log_level_t register_log_level(const std::string &name) {
            if(impl::max_predefined_log_level() == INT_MAX)
                throw std::overflow_error("At most {}(INT_MAX) log_level is allowed."_format(INT_MAX));
            ++ impl::max_predefined_log_level();
            log_level_t new_level = (log_level_t)impl::max_predefined_log_level();
            custom_log_level_names.push_back({new_level, name});
            return new_level;
        }

#define RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND(_name, _enum_name) template <typename ... Args> void _name(const std::string &info, Args ... extra) const { \
            log(log_level_t::_enum_name, info, std::forward<Args>(extra) ...); }

        RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND(fatal, FATAL)
        RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND(error, ERROR)
        RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND(warning, WARNING)
        RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND(info, INFO)
        RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND(verbose, VERBOSE)
        RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND(debug, DEBUG)

#undef RLIB_IMPL_MACRO_LOG_ADD_SHORTHAND

    private:
        static constexpr const char * predefined_log_level_name(log_level_t level) noexcept {
            switch(level) {
            case log_level_t::FATAL:
                return "FATAL";
            case log_level_t::ERROR:
                return "ERROR";
            case log_level_t::WARNING:
                return "WARNING";
            case log_level_t::INFO:
                return "INFO";
            case log_level_t::VERBOSE:
                return "VERBOSE";
            case log_level_t::DEBUG:
                return "DEBUG";
            default:
                return "";
            }
        }
        static constexpr bool is_predefined_log_level(log_level_t level) noexcept {
            return predefined_log_level_name(level)[0] != '\0';
        }
        std::string log_level_name(log_level_t level) const noexcept {
            std::string name = predefined_log_level_name(level);
            if(!name.empty())
                return name;
            for(const auto &level_and_name : custom_log_level_names) {
                if(level == level_and_name.first) {
                    name = level_and_name.second;
                    break;
                }
            }
            if(!name.empty())
                return name;
            name = "LEVEL-";
            name += std::to_string((int)level);
            return name;
        }

        std::ostream *pstream;

        std::list<std::pair<log_level_t, std::string> > custom_log_level_names;
        log_level_t log_level = log_level_t::INFO; // `Ignore` deadline.

        bool must_delete_stream_as_ofstream = false;
        bool enable_flush = true;
#if RLIB_IMPL_ENABLE_LOGGER_FROM_FD == 1
#   if RLIB_COMPILER_ID == CC_GCC
        __gnu_cxx::stdio_filebuf<char> _gcc_filebuf;
        std::ostream _gcc_real_stream;
#   elif RLIB_COMPILER_ID == CC_MSVC
        std::ofstream _msvc_real_stream;
#   endif
#endif
    };
}

#endif
