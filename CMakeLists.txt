cmake_minimum_required(VERSION 3.17)
project(epicalyx)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        epicalyx
        main.cpp
)
if (MSVC)
    add_compile_options(/EHa)
else()
    add_compile_options(-fexception)
endif()

add_subdirectory(src/tokenizer)

target_link_libraries(epicalyx Tokenizer)