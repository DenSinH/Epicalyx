#ifndef EPICALYX_TOKENIZER_H
#define EPICALYX_TOKENIZER_H

#include "tokens.h"
#include "../state/state.h"

#include <string>
#include <vector>
#include <memory>

class Tokenizer : public Stateful {

public:
    Tokenizer() = default;

    void Tokenize(const std::string& file_name);

    std::vector<TOKEN> Tokens;

    std::string File;
    size_t LineNo;
    std::string Line;

private:
    template<typename T, typename... Args>
    TOKEN MakeToken(const Args&... args) {
        return MAKE_TOKEN(T)(File, LineNo, Line, args...);
    }

    void TokenizeLine(std::ifstream& file, const std::string& line);
    TOKEN ReadNumericConstant(std::string::const_iterator& current, const std::string::const_iterator& end);
    TOKEN ReadStringConstant(std::string::const_iterator& current, const std::string::const_iterator& end);
    TOKEN ReadCharSequenceConstant(std::string::const_iterator& current, const std::string::const_iterator& end);
    static unsigned char ReadCChar(std::string::const_iterator& current, const std::string::const_iterator& end);
};

#endif //EPICALYX_TOKENIZER_H
