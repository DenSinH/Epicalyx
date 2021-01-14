#ifndef EPICALYX_TOKENIZER_H
#define EPICALYX_TOKENIZER_H

#include "tokens.h"

#include <string>
#include <vector>
#include <variant>


class Tokenizer {

public:
    Tokenizer() = default;

    void Tokenize(const std::string& file_name);

    /*
     * later:
       std::variant<Parent, Child> var;
       var = Parent{...};
       auto parent = std::get<Parent>(var);
       var = Child{...};
       auto child = std::get<Child>(var);
       // This will throw an exception
       auto invalid = std::get<Parent>(var);
       // You can check if the variant holds an specific type like this:
       if (std::holds_alternative<Parent>(var)) {
         // Safe to do this here
         std::get<Parent>(var);
       }
     * */
    std::vector<std::variant<
        Token,
        Identifier,
        StringConstant,
        NumericalConstant<double>,
        NumericalConstant<unsigned long long>
    >> Tokens;

private:
    void TokenizeLine(std::ifstream& file, const std::string& line);
    static std::variant<NumericalConstant<double>, NumericalConstant<unsigned long long>> ReadNumericConstant(std::string::const_iterator& current, std::string::const_iterator end, std::string& dest);
    static void ReadStringConstant(std::string::const_iterator& current, std::string::const_iterator end, std::string& dest);
    static NumericalConstant<unsigned long long> ReadCharSequenceConstant(std::string::const_iterator& current, std::string::const_iterator end);
    static unsigned char ReadCChar(std::string::const_iterator& current, std::string::const_iterator end);
};

#endif //EPICALYX_TOKENIZER_H
