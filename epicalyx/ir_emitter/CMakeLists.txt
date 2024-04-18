add_library(IREmitter STATIC
        Emitter.cpp
        Emitter.h
        ASTWalker.cpp
        ASTWalker.h
)

target_precompile_headers(IREmitter REUSE_FROM CalyxHeaders)