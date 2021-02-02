add_library(Parser STATIC
        parser.h
        AST.h
        parser_state.h

        types/types.h

        parser_utils.cpp
        parser.cpp

        expression/expression_nodes.cpp
        expression/parser_expression.cpp

        declaration/parser_declaration.cpp

        statement/parser_statement.cpp

        expression/expression_nodes.h
        expression/primary_expression.h
        expression/postfix_expression.h
        expression/unary_expression.h
        expression/cast_expression.h
        expression/binop_expression.h
        expression/expression.h
        expression/assign_expression.h
        expression/cond_expression.h
        expression/tables.cpp

        declaration/declaration_nodes.h
        declaration/specifiers.h
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

        statement/statement_nodes.h
        statement/labeled.h
        statement/compound.h
        statement/selection.h
        statement/jump.h
        statement/iteration.h
        statement/empty.h

        translation_unit/function_definition.h
        translation_unit/parser_translation_unit.cpp
)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(Parser PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/include)

add_subdirectory(analysis)
target_link_libraries(Parser Analysis)