add_library(Nodes STATIC
        Expression.h Expression.cpp)

set_target_properties(Nodes PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Nodes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})