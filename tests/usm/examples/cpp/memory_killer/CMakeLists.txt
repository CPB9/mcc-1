cmake_minimum_required(VERSION 2.8.11)

project(memorykiller)

set(MEMORYKILLER_SOURCES
    src/memorykiller.cpp
    )

add_executable(memorykiller ${MEMORYKILLER_SOURCES})

if(NOT MSVC)
    target_compile_options(memorykiller PUBLIC -std=c++11)
    target_compile_options(memorykiller PRIVATE -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual)
endif()

install (TARGETS ${PROJECT_NAME} DESTINATION bin)

