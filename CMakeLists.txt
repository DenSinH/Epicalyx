cmake_minimum_required(VERSION 3.17)
project(epicalyx)

set(CMAKE_CXX_STANDARD 20)

set( CMAKE_VERBOSE_MAKEFILE on )

add_executable(
        epicalyx
        main.cpp
        src/include/flags.h
        src/include/log.h
        src/include/utils.h
        src/state/state.h
        src/file/file.h
)

if (MSVC)
    add_compile_options(/EHsc)
else()
    add_compile_options(-fexceptions)
endif()

# add_compile_options(-fstack-protector)
add_subdirectory(src/tokenizer)
add_subdirectory(src/parser)

target_link_libraries(epicalyx Tokenizer)
target_link_libraries(epicalyx Parser)