add_library(Tokenizer STATIC
        Token.h
        Tokenizer.h
        Maps.cpp
        Tokenizer.cpp)

set_target_properties(Tokenizer PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Tokenizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})