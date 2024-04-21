add_library(Types STATIC
        BaseType.h
        BaseType.cpp
        Types.h 
        Types.cpp
        AnyType.h
        AnyType.cpp
        TypeFwd.h)

set_target_properties(Types PROPERTIES LINKER_LANGUAGE CXX)
target_precompile_headers(Types REUSE_FROM epicalyx-config)
target_include_directories(Types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})