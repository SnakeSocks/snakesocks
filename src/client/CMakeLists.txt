cmake_minimum_required(VERSION 3.5)
project(SnakeSocksCli)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DCOMPILE_SS_MAIN_CLIENT)
add_definitions(-D___NO_THREAD_POOL)

set(CMAKE_CXX_FLAGS_DEBUG "-g -DMALLOC_CHECK_=2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

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
add_executable(skcli ${CLIENT_SRC})
target_link_libraries(skcli Threads::Threads)
target_link_libraries(skcli Boost::system)
target_link_libraries(skcli dl)

#if(FOR_M32)
#    set_target_properties(skcli PROPERTIES COMPILE_FLAGS -m32 LINK_FLAGS -m32 )
#endif(FOR_M32)
