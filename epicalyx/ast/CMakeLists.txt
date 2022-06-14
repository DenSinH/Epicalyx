add_library(AST STATIC
        Expression.h
        Expression.cpp
        Node.h
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