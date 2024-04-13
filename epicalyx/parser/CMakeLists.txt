add_library(Parser STATIC
        Parser.h
        Parser.Expressions.cpp
        Parser.Declarations.cpp
        Parser.Statements.cpp
        Parser.cpp
        ConstParser.h
)

set_target_properties(Parser PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})