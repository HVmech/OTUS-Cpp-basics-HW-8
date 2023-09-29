cmake_minimum_required(VERSION 3.5)

project(WordCounter)

add_executable(word_counter
    main.cpp
    src/mtq.ipp
)

# Add include to the include directories for the test target
target_include_directories(
    word_counter PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(word_counter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if (MSVC)
    target_compile_options(
        word_counter PRIVATE
        /EHa /W4 /WX
    )
else ()
    target_compile_options(
        word_counter PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()