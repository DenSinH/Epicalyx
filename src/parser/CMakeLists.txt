add_library(Parser STATIC
        parser.h
        parser.cpp
        AST.h
        expression/primary_expression.h expression/postfix_expression.h expression/unary_expression.h expression/cast_expression.h)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(Parser PROPERTIES LINKER_LANGUAGE CXX)


target_include_directories(Parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/include)