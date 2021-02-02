add_library(Tokenizer STATIC
        tokenizer.h
        tokenizer.cpp
        tokens.h
        punctuators.h
        keywords.h
        token_types.h
        strings.cpp
        numerical.cpp
        tokens.cpp)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(Tokenizer PROPERTIES LINKER_LANGUAGE CXX)


target_include_directories(Tokenizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/include)