#ifndef EPICALYX_TOKENIZER_H
#define EPICALYX_TOKENIZER_H

#include "tokens.h"

#include <string>
#include <vector>
#include <memory>


class Tokenizer {

public:
    Tokenizer() = default;

    void Tokenize(const std::string& file_name);

    std::vector<std::unique_ptr<Token>> Tokens;

private:
    void TokenizeLine(std::ifstream& file, const std::string& line);
};

#endif //EPICALYX_TOKENIZER_H
