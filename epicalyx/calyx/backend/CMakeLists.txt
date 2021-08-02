add_library(CalyxBackend STATIC
        Backend.h Example.h Example.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(interpreter)

target_link_libraries(CalyxBackend CalyxInterpreter)