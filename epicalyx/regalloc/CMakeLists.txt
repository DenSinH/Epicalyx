add_library(RegAlloc STATIC
        Allocator.h
        Allocator.cpp
        RIG.h
        RIG.cpp)

set_target_properties(RegAlloc PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(RegAlloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})