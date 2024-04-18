include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(CalyxBackend STATIC
        Example.h Example.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(CalyxBackend REUSE_FROM CalyxHeaders)

add_subdirectory(interpreter)

target_link_libraries(CalyxBackend CalyxInterpreter)