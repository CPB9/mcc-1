cmake_minimum_required(VERSION 2.8.11)

project(longbuild)

set(LONGBUILD_SOURCES
    src/longbuild.cpp
    )

add_executable(longbuild ${LONGBUILD_SOURCES})

if(NOT MSVC)
    target_compile_options(longbuild PUBLIC -std=c++11)
    target_compile_options(longbuild PRIVATE -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual)
endif()

install (TARGETS ${PROJECT_NAME} DESTINATION bin)

