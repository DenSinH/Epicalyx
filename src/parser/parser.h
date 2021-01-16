#ifndef EPICALYX_PARSER_H
#define EPICALYX_PARSER_H

#include "../tokenizer/tokenizer.h"

class Parser {
public:


private:
    std::vector<std::variant<
            Token,
            Identifier,
            StringConstant,
            NUMERICAL_CONSTANTS
    >> Tokens;
};

#endif //EPICALYX_PARSER_H
