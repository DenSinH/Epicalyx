add_library(AST STATIC
        Expression.h
        Expression.cpp
        Node.h
        NodeFwd.h
        Declaration.h
        Statement.cpp
        Statement.h
        Declaration.cpp
        Initializer.cpp
        Initializer.h
        Node.cpp
        NodeVisitor.h
)

set_target_properties(AST PROPERTIES LINKER_LANGUAGE CXX)
target_precompile_headers(AST REUSE_FROM epicalyx-config)