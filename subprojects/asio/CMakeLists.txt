project(asio)

cmake_minimum_required(VERSION 2.8.11)

find_package(Threads)

if(MSVC)
    set(ASIO_FLAGS -DASIO_SEPARATE_COMPILATION -DASIO_STANDALONE -D_WIN32_WINNT=0x0601 -DASIO_ENABLE_CANCELIO)
else()
    set(ASIO_FLAGS -DASIO_SEPARATE_COMPILATION -DASIO_STANDALONE)
endif()

add_library(asio
    ${CMAKE_CURRENT_SOURCE_DIR}/asio/src/asio.cpp
)

target_link_libraries(asio ${CMAKE_THREAD_LIBS_INIT})

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(HAS_PARENT_SCOPE)
    set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/asio/include PARENT_SCOPE)
    set(ASIO_LIBRARIES asio PARENT_SCOPE)
endif()

target_compile_options(asio PUBLIC ${ASIO_FLAGS})
if (NOT MSVC)
    target_compile_options(asio PRIVATE -std=c++11)
endif()
target_include_directories(asio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/asio/include)
