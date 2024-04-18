add_library(CalyxInterpreter STATIC
        Interpreter.h Interpreter.cpp)

target_precompile_headers(CalyxInterpreter REUSE_FROM CalyxHeaders)