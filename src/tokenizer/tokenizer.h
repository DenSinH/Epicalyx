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

    std::vector<TOKEN> Tokens;

private:
    void TokenizeLine(std::ifstream& file, const std::string& line);
    static TOKEN ReadNumericConstant(std::string::const_iterator& current, std::string::const_iterator end);
    static TOKEN ReadStringConstant(std::string::const_iterator& current, std::string::const_iterator end);
    static TOKEN ReadCharSequenceConstant(std::string::const_iterator& current, std::string::const_iterator end);
    static unsigned char ReadCChar(std::string::const_iterator& current, std::string::const_iterator end);
};

#endif //EPICALYX_TOKENIZER_H
