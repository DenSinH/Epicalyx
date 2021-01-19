add_library(Parser STATIC
        parser.h
        parser_expressions.cpp
        AST.h
        parser_utils.cpp
        expression/primary_expression.h
        expression/postfix_expression.h
        expression/unary_expression.h
        expression/cast_expression.h
        expression/binop_expression.h
        expression/expression.h
        expression/assign_expression.h
        expression/cond_expression.h
)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(Parser PROPERTIES LINKER_LANGUAGE CXX)


target_include_directories(Parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/include)