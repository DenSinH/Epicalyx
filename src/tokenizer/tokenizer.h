#ifndef EPICALYX_TOKENIZER_H
#define EPICALYX_TOKENIZER_H

#include "tokens.h"
#include "../state/state.h"
#include "../file/file.h"

#include <string>
#include <vector>
#include <memory>

class Tokenizer : public Stateful {

public:
    Tokenizer() = default;

    void Tokenize(std::shared_ptr<const File> file);

    std::vector<TOKEN> Tokens;

    std::shared_ptr<const File> FileObj;
    size_t LineNo;
    std::shared_ptr<const std::string> Line;

private:
    std::string current_token;  // keep this here to know it in case of an error

    template<typename T, typename... Args>
    TOKEN MakeToken(const Args&... args) {
        return MAKE_TOKEN(T)(FileObj->FileName, LineNo, Line, args...);
    }

    void TokenizeLine(const std::string& line);
    TOKEN ReadNumericConstant(std::string::const_iterator& current, const std::string::const_iterator& end);
    TOKEN ReadStringConstant(std::string::const_iterator& current, const std::string::const_iterator& end);
    TOKEN ReadCharSequenceConstant(std::string::const_iterator& current, const std::string::const_iterator& end);
    static unsigned char ReadCChar(std::string::const_iterator& current, const std::string::const_iterator& end);
};

#endif //EPICALYX_TOKENIZER_H
