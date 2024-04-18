add_library(RegAlloc STATIC
        GeneralizedVar.h
        Allocator.h
        Allocator.cpp
        RIG.h
        RIG.cpp
        RegisterSpace.h
        RegisterSpace.cpp
        regspaces/Example.h
        regspaces/Example.cpp
)

set_target_properties(RegAlloc PROPERTIES LINKER_LANGUAGE CXX)
target_precompile_headers(RegAlloc REUSE_FROM CalyxHeaders)
target_include_directories(RegAlloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})