add_library(Tokenizer STATIC
        Token.h
        TokenType.h
        Tokenizer.h
        Maps.cpp
        Tokenizer.cpp
        Token.cpp
        Preprocessor.cpp Preprocessor.h Identifier.h)

set_target_properties(Tokenizer PROPERTIES LINKER_LANGUAGE CXX)
target_precompile_headers(Tokenizer REUSE_FROM epicalyx-config)
target_include_directories(Tokenizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})