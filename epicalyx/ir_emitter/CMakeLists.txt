add_library(IREmitter STATIC
        Emitter.cpp
        Emitter.h
        ASTWalker.cpp
        ASTWalker.Declaration.cpp
        ASTWalker.Expression.cpp
        ASTWalker.Statement.cpp
        ASTWalker.h
        Helpers.h
        Helpers.cpp
        Helpers.inl
)

target_precompile_headers(IREmitter REUSE_FROM CalyxHeaders)