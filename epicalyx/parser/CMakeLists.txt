add_library(Parser STATIC
        Parser.h Parser.cpp ConstTokenVisitor.h ConstTokenVisitor.cpp)

set_target_properties(Parser PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(nodes)

target_link_libraries(Parser Nodes)