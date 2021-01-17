#ifndef EPICALYX_PARSER_H
#define EPICALYX_PARSER_H

#include "../tokenizer/tokenizer.h"


class Parser {
public:
    explicit Parser(TokenVector& tokens) {
        this->Tokens = tokens;
    }

    explicit Parser(Tokenizer& tokenizer) {
        this->Tokens = tokenizer.Tokens;
    }

private:
    TokenVector Tokens;
};

#endif //EPICALYX_PARSER_H
