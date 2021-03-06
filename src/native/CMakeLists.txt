cmake_minimum_required(VERSION 3.5)
project(SnakeSocksNative)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DCOMPILE_SS_MAIN_CLIENT)
add_definitions(-D___NO_THREAD_POOL)
add_definitions(-DSK_VERSION=1.4.0)

set(CMAKE_CXX_FLAGS_DEBUG "-g -DMALLOC_CHECK_=2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-user-defined-literals")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-literal-suffix -Wno-terminate")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

string(TIMESTAMP _COMPILE_TIME %Y-%m-%dT%H:%M:%SZ UTC)
add_definitions(-DCOMPILE_TIME=${_COMPILE_TIME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# MacOS has no MSG_NOSIGNAL define? I'm not sure if this will lead to EPIPE.
if(APPLE)
    add_definitions(-DMSG_NOSIGNAL=0)
endif()

include_directories(./src)
include_directories(./src/lib)

set(Boost_USE_STATIC_LIBS ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED system)

include_directories(${Boost_INCLUDE_DIRS})

set(CLIENT_SRC src/client.cc src/socks5.cc src/tunnel.cc src/module.cc src/lib/NetLib.cc)
set(SERVER_SRC src/server.cc src/socks5.cc src/tunnel.cc src/module.cc src/lib/NetLib.cc)

add_executable(skcli ${CLIENT_SRC})
target_link_libraries(skcli -static-libgcc -static-libstdc++)
target_link_libraries(skcli Threads::Threads Boost::system dl)

add_executable(sksrv ${SERVER_SRC})
target_link_libraries(sksrv -static-libgcc -static-libstdc++)
target_link_libraries(sksrv Threads::Threads Boost::system dl)

#if(FOR_M32)
#    set_target_properties(skcli PROPERTIES COMPILE_FLAGS -m32 LINK_FLAGS -m32 )
#endif(FOR_M32)
