#ifndef EPICALYX_TOKENIZER_H
#define EPICALYX_TOKENIZER_H

#include "tokens.h"

#include <string>
#include <vector>
#include <variant>

#define NUMERICAL_CONSTANTS \
NumericalConstant<int> \
, NumericalConstant<unsigned int>     \
, NumericalConstant<long> \
, NumericalConstant<unsigned long> \
, NumericalConstant<long long> \
, NumericalConstant<unsigned long long> \
, NumericalConstant<float> \
, NumericalConstant<double> \


typedef std::vector<std::variant<
        Token,
        Identifier,
        StringConstant,
        NUMERICAL_CONSTANTS
>> TokenVector;

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
    TokenVector Tokens;

private:
    void TokenizeLine(std::ifstream& file, const std::string& line);
    static std::variant<NUMERICAL_CONSTANTS> ReadNumericConstant(std::string::const_iterator& current, std::string::const_iterator end, std::string& dest);
    static void ReadStringConstant(std::string::const_iterator& current, std::string::const_iterator end, std::string& dest);
    static NumericalConstant<unsigned long long> ReadCharSequenceConstant(std::string::const_iterator& current, std::string::const_iterator end);
    static unsigned char ReadCChar(std::string::const_iterator& current, std::string::const_iterator end);
};

#endif //EPICALYX_TOKENIZER_H
