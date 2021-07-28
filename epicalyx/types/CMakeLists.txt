add_library(Types STATIC
        Types.h Types.cpp EpiCType.h)

set_target_properties(Types PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})