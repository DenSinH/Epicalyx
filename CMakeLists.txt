cmake_minimum_required(VERSION 3.28)
project(epicalyx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# set( CMAKE_VERBOSE_MAKEFILE on )

# configure ccache for better compile times
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "Found ccache, using for optimized compilation")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

# find boost
find_package(Boost 1.75.0)

# it is possible to build without boost
if(${Boost_FOUND})
    message(STATUS "Found Boost at ${Boost_INCLUDE_DIR}")
    include_directories(epicalyx ${Boost_INCLUDE_DIR})
    add_compile_options(-DUSE_BOOST)
else()
  message(STATUS "Boost not found, using stl. Please set BOOST_ROOT (or BOOSTROOT) environment variables.")
endif()

# set up include directories
include_directories(
  ${PROJECT_SOURCE_DIR}/epicalyx 
  ${PROJECT_SOURCE_DIR}/epicalyx/include
)

# set up exceptions
if (MSVC)
    add_compile_options(/EHsc)
else()
    add_compile_options(-fexceptions)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        # message(STATUS "Adding time trace for compilation")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -ftime-trace -fno-pie")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    endif()
endif()
add_compile_options(-Wno-format-security)

# project common include headers
set(INCLUDE_HEADERS 
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Locatable.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/CString.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Decltype.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Stream.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/TypeTraits.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Variant.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/TypeName.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Packs.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Stringify.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Format.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Is.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Scope.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Log.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Escape.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Cast.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/CustomAssert.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Containers.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Vector.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Hash.h
  ${PROJECT_SOURCE_DIR}/epicalyx/include/Exceptions.h
)

# config library with PCH for most subfolders
add_library(
  epicalyx-config
  STATIC
  epicalyx/config/Fake.cpp
  epicalyx/config/PCH.h
)

target_precompile_headers(
  epicalyx-config
  PUBLIC
  epicalyx/config/PCH.h
)

# main executable
add_executable(
        epicalyx
        main.cpp
        epicalyx/config/Log.cpp
        epicalyx/config/Info.h
        epicalyx/config/argparse/argparse.hpp
        epicalyx/config/Info.cpp
        ${INCLUDE_HEADERS}
)

target_link_libraries(epicalyx epicalyx-config)

# link subdirectories

# add_compile_options(-fstack-protector)
add_subdirectory(epicalyx/file)
add_subdirectory(epicalyx/tokenizer)
add_subdirectory(epicalyx/types)
add_subdirectory(epicalyx/ast)
add_subdirectory(epicalyx/parser)
add_subdirectory(epicalyx/cycle)

target_link_libraries(epicalyx File)
target_link_libraries(epicalyx Tokenizer)
target_link_libraries(epicalyx Types)
target_link_libraries(epicalyx AST)
target_link_libraries(epicalyx Parser)
target_link_libraries(epicalyx Cycle)


# uses Calyx I
add_subdirectory(epicalyx/calyx)
add_subdirectory(epicalyx/ir_emitter)
add_subdirectory(epicalyx/optimizer)
add_subdirectory(epicalyx/regalloc)

target_link_libraries(epicalyx Calyx)
target_link_libraries(epicalyx IREmitter)
target_link_libraries(epicalyx Optimizer)
target_link_libraries(epicalyx RegAlloc)

target_precompile_headers(epicalyx REUSE_FROM CalyxHeaders)

add_custom_target(graphviz ALL
    COMMAND ${CMAKE_COMMAND} "--graphviz=epicalyx.dot" .
    COMMAND dot -Tpdf epicalyx.dot -o epicalyx.pdf
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/_graphviz"
)

add_test(
    NAME scctests
    COMMAND 
    python "run_suite.py" 
          "$<TARGET_FILE:epicalyx>"
          "./scctests/cc/execute"
          "./scctests/output.txt"
          "./scctests/errors.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests/suites"
)

# we are not using any boost libraries that need compiling
# if(${Boost_FOUND})
#     target_link_libraries(epicalyx ${Boost_LIBRARY})
# endif()