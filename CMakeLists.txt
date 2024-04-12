cmake_minimum_required(VERSION 3.17)
project(epicalyx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# set( CMAKE_VERBOSE_MAKEFILE on )

find_package(Boost 1.75.0)

if(${Boost_FOUND})
    message(STATUS "Found Boost at ${Boost_INCLUDE_DIR}")
    include_directories(epicalyx ${Boost_INCLUDE_DIR})
    add_compile_options(-DUSE_BOOST)
else()
  message(STATUS "Boost not found, using stl. Please set BOOST_ROOT (or BOOSTROOT) environment variables.")
endif()

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
        epicalyx/include/Variant.h
        epicalyx/include/Packs.h
        epicalyx/include/Format.h
        epicalyx/include/Is.h
        epicalyx/include/Scope.h
        epicalyx/include/Log.h
        epicalyx/include/Log.cpp
        epicalyx/include/Escape.h
        epicalyx/include/Cast.h
        epicalyx/include/CustomAssert.h
        epicalyx/include/Containers.h
        epicalyx/include/Hash.h
        epicalyx/include/Algorithm.h 
        epicalyx/include/Exceptions.h)



# add_compile_options(-fstack-protector)
add_subdirectory(epicalyx/file)
add_subdirectory(epicalyx/tokenizer)
add_subdirectory(epicalyx/types)
add_subdirectory(epicalyx/ast)
add_subdirectory(epicalyx/parser)
add_subdirectory(epicalyx/calyx)
add_subdirectory(epicalyx/cycle)
add_subdirectory(epicalyx/ir_emitter)
add_subdirectory(epicalyx/optimizer)
add_subdirectory(epicalyx/regalloc)

target_link_libraries(epicalyx File)
target_link_libraries(epicalyx Tokenizer)
target_link_libraries(epicalyx Types)
target_link_libraries(epicalyx AST)
target_link_libraries(epicalyx Parser)
target_link_libraries(epicalyx Calyx)
target_link_libraries(epicalyx Cycle)
target_link_libraries(epicalyx IREmitter)
target_link_libraries(epicalyx Optimizer)
target_link_libraries(epicalyx RegAlloc)

# we are not using any boost libraries that need compiling
# if(${Boost_FOUND})
#     target_link_libraries(epicalyx ${Boost_LIBRARY})
# endif()