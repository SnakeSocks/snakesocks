# use this file if you want to install rlib

set(rlib_INCLUDE_DIRS ${PREFIX}/include)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # using clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(rlib_CXX_FLAGS "-Wno-terminate")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    add_definitions(-DRLIB_MINGW_DISABLE_TLS)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
endif()

if(MINGW)
    add_definitions(-DRLIB_MINGW_DISABLE_TLS)
endif()
