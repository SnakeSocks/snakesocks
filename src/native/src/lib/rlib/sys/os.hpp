#ifndef R_OS_HPP
#define R_OS_HPP

#ifndef RLIB_OS_ID
#if defined(_Windows) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32)
#   define RLIB_OS_ID OS_WINDOWS
#elif defined(__linux__) || defined(__linux)
#   define RLIB_OS_ID OS_LINUX
#elif defined(__APPLE__)
#   include "TargetConditionals.h"
#   if TARGET_IPHONE_SIMULATOR
#   define RLIB_OS_ID OS_IOS
#   elif TARGET_OS_IPHONE
#   define RLIB_OS_ID OS_IOS
#   elif TARGET_OS_MAC
#   define RLIB_OS_ID OS_MACOS
#   else
#   define RLIB_OS_ID OS_UNKNOWN
#   endif
#elif defined(__OS_ANDROID__)
#   define RLIB_OS_ID OS_ANDROID
#elif defined(__unix__) || defined(__unix)
#   define RLIB_OS_ID OS_UNIX
#else
#   define RLIB_OS_ID OS_UNKNOWN
#endif
#endif

#define RLIB_OS_ID_MAGIC 980427
#define OS_WINDOWS (RLIB_OS_ID_MAGIC + 1) 
#define OS_LINUX   (RLIB_OS_ID_MAGIC + 2) 
#define OS_MACOS   (RLIB_OS_ID_MAGIC + 3) 
#define OS_BSD     (RLIB_OS_ID_MAGIC + 4) 
#define OS_IOS     (RLIB_OS_ID_MAGIC + 5) 
#define OS_ANDROID (RLIB_OS_ID_MAGIC + 6) 
#define OS_UNIX    (RLIB_OS_ID_MAGIC + 7) 
#define OS_UNKNOWN (RLIB_OS_ID_MAGIC + 8) 

#include "compiler_detector"
// Define RLIB_COMPILER_ID and RLIB_COMPILER_VER

#if defined(__MINGW32__) || defined(__MINGW64__)
#define RLIB_COMPILER_IS_MINGW 1
#else
#define RLIB_COMPILER_IS_MINGW 0
#endif

// shorthand for __cplusplus macro.
#ifndef RLIB_CXX_STD
#   if RLIB_COMPILER_ID == CC_MSVC
#       if defined(__cplusplus)
#       if __cplusplus == 199711L
#           pragma message (": warning MSVC_Wrong__cplusplus: Your MSVC is possibly set __cplusplus to wrong value. Please upgrade to VS2017 15.7 Preview 3 and recompile with /Zc:__cplusplus. Or I'll assume you support C++17 and set RLIB_CXX_STD to 2017. (refer to https://blogs.msdn.microsoft.com/vcblog/2018/04/09/msvc-now-correctly-reports-__cplusplus/)")
#           define RLIB_CXX_STD 2017
#       else
#           define RLIB_CXX_STD (__cplusplus / 100L)
#       endif
#       endif
#   else
#       if defined(__cplusplus)
#       define RLIB_CXX_STD (__cplusplus / 100L)
#       endif
#   endif
#endif

#if RLIB_CXX_STD >= 2011
namespace rlib {
    class os_info
    {
    public:
        enum class os_t {UNKNOWN = OS_UNKNOWN, WINDOWS = OS_WINDOWS, LINUX = OS_LINUX, MACOS = OS_MACOS, BSD = OS_BSD, IOS = OS_IOS, ANDROID = OS_ANDROID, UNIX = OS_UNIX};
        enum class compiler_t {UNKNOWN = CC_UNKNOWN, GCC = CC_GCC, CLANG = CC_CLANG, MSVC = CC_MSVC, ICC = CC_ICC, BORLAND = CC_BORLAND, IARC = CC_IARC, SOLARIS = CC_SOLARIS/*, ZAPCC = CC_ZAPCC*/};
        //C = CC_Compiler which not supports cxx1x yet is not listed here. 201708.
        
        static constexpr os_t os = 
    #if defined(RLIB_OS_ID)
    (os_t)RLIB_OS_ID;
    #else
    os_t::UNKNOWN;
    #endif
        static constexpr compiler_t compiler = 
    #if defined(RLIB_COMPILER_ID)
    (compiler_t)RLIB_COMPILER_ID;
    #else
    compiler_t::UNKNOWN;
    #endif
        static constexpr auto compiler_version = 
    #if defined(RLIB_COMPILER_VER)
    RLIB_COMPILER_VER;
    #else
    0;
    #endif
    };
}

#endif

#endif
