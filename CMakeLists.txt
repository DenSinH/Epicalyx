cmake_minimum_required(VERSION 3.17)
project(epicalyx)

set(CMAKE_CXX_STANDARD 20)

# set( CMAKE_VERBOSE_MAKEFILE on )

include_directories(epicalyx epicalyx/include)

if (MSVC)
    add_compile_options(/EHsc)
else()
    add_compile_options(-fexceptions)
endif()
add_compile_options(-Wno-format-security)

add_executable(
        epicalyx
        main.cpp
        epicalyx/include/Stream.h
        epicalyx/include/Format.h
        epicalyx/file/File.cpp
        epicalyx/file/File.h
        epicalyx/include/Is.h
        epicalyx/include/Scope.h
        epicalyx/include/Log.h
        epicalyx/include/Log.cpp
        epicalyx/include/Escape.h epicalyx/include/Cast.h)



# add_compile_options(-fstack-protector)
add_subdirectory(epicalyx/file)
add_subdirectory(epicalyx/tokenizer)
add_subdirectory(epicalyx/types)
add_subdirectory(epicalyx/taxy)
add_subdirectory(epicalyx/parser)
add_subdirectory(epicalyx/calyx)
add_subdirectory(epicalyx/phyte)

target_link_libraries(epicalyx File)
target_link_libraries(epicalyx Tokenizer)
target_link_libraries(epicalyx Types)
target_link_libraries(epicalyx Taxy)
target_link_libraries(epicalyx Parser)
target_link_libraries(epicalyx Calyx)
target_link_libraries(epicalyx Phyte)