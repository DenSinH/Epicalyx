add_library(Types STATIC
        Types.h Types.cpp)

set_target_properties(Types PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})