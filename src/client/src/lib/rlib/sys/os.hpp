#ifndef R_OS_HPP
#define R_OS_HPP

#ifndef __OS_ID__
#if defined(_Windows) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32)
#   define __OS_ID__ WINDOWS
#elif defined(__linux__) || defined(__linux)
#   define __OS_ID__ LINUX
#elif defined(__APPLE__)
#   include "TargetConditionals.h"
#   if TARGET_IPHONE_SIMULATOR
#   define __OS_ID__ IOS
#   elif TARGET_OS_IPHONE
#   define __OS_ID__ IOS
#   elif TARGET_OS_MAC
#   define __OS_ID__ MACOS
#   else
#   define __OS_ID__ UNKNOWN_UNIX
#   endif
#elif defined(__ANDROID__)
#   define __OS_ID__ ANDROID
#elif defined(__unix__) || defined(__unix)
#   define __OS_ID__ UNKNOWN_UNIX
#else
#   define __OS_ID__ UNKNOWN
#endif
#endif

#include "compiler_detector"
// Define __COMPILER_ID__ and __COMPILER_VER__

#if __cplusplus >= 201103L

class OSInfo
{
public:
    enum class os_t {UNKNOWN, WINDOWS, LINUX, MACOS, BSD, IOS, ANDROID, UNKNOWN_UNIX};
    enum class compiler_t {UNKNOWN, GCC, CLANG, MSVC, INTELC, BORLAND, IARC, SOLARIS, ZAPCC}; //Compiler which not supports cxx1x yet is not listed here. 201708.
#if defined(__OS_ID__)
    static constexpr os_t os = os_t::__OS_ID__;
#endif
#if defined(__COMPILER_ID__)
    static constexpr compiler_t compiler = compiler_t::__COMPILER_ID__;
#endif
#if defined(__COMPILER_VER__)
    static constexpr auto compiler_version = __COMPILER_VER__;
#else
    static constexpr auto compiler_version = 0;
#endif
};

#endif

#endif
