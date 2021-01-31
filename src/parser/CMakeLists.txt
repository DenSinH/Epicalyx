add_library(Parser STATIC
        parser.h
        parser_expression.cpp
        parser_declaration.cpp
        parser_statement.cpp
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
        declaration/specifiers.h
        expression/tables.cpp
        declaration/enum.h
        declaration/struct.h
        declaration/tables.cpp
        declaration/static_assert.h
        declaration/designation.h
        declaration/initializer.h
        declaration/declaration.h
        declaration/declarator.h
        declaration/pointer.h
        declaration/typename.h
        declaration_nodes.h
        expression_nodes.h
        statement/labeled.h
        statement_nodes.h
        statement/compound.h
        statement/selection.h
        statement/jump.h
        statement/iteration.h statement/empty.h translation_unit/function_definition.h parser_translation_unit.cpp Parser.cpp parser.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(Parser PROPERTIES LINKER_LANGUAGE CXX)


target_include_directories(Parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/include)