cmake_minimum_required(VERSION 3.17)
project(epicalyx)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        epicalyx
        main.cpp
        src/include/flags.h
        src/include/log.h
        src/utils.h)

if (MSVC)
    add_compile_options(/EHa)
else()
    add_compile_options(-fexceptions)
endif()

# add_compile_options(-fstack-protector)
add_subdirectory(src/tokenizer)
add_subdirectory(src/parser)

target_link_libraries(epicalyx Tokenizer)
target_link_libraries(epicalyx Parser)