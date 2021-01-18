#include "tokenizer.h"
#include <climits>

enum class NumberType {
    Decimal,
    Octal,
    Hex,
    DecimalFloat,
    HexFloat,
};

std::shared_ptr<Token> Tokenizer::ReadNumericConstant(
        std::string::const_iterator& current,
        std::string::const_iterator end
) {
    std::string value;
    NumberType type = NumberType::Decimal;
    bool dot      = false;
    bool exponent = false;
    bool hex      = false;
    bool is_float = false;

    auto next = current + 1;
    // we already know that we are starting with a digit
    value.push_back(*current);

    if (*current == '0') {
        if ((current != end) && (*next == 'x' || *next == 'X')) {
            // hex-prefex
            type = NumberType::Hex;
            value.push_back('x');
            hex = true;
            current++;
        }
        else {
            // octal prefix
            type = NumberType::Octal;
        }
    }
    else if (*current == '.') {
        value.push_back(*current);
        current++;
        dot = true;
        type = NumberType::DecimalFloat;
    }

    // first digit already appended
    current++;
    while (true) {
        // keep reading digits
        while (current != end
               && ((!hex && std::isdigit(*current)) || (hex && std::isxdigit(*current)))) {
            value.push_back(*current);
            current++;
        }

        // we might have a dot for a float / double
        if (current != end) {
            if (!dot && (*current == '.')) {
                // (first) dot, number is a decimal float, other types of floats may not contain dots
                if (hex) { break; } // dots not allowed in hex float

                value.push_back(*current);
                current++;

                is_float = true;

                type = NumberType::DecimalFloat;

                if (current == end || !std::isdigit(*current)) {
                    // char after dot has to be number or EOF to continue searching
                    break;
                }
            }
            else if (!exponent && (*current == 'e' || *current == 'E')) {
                // exponent, number is a decimal float
                if (hex) { break; } // exponents not allowed in hex floats

                value.push_back(*current);
                current++;

                // don't allow more dots
                dot      = true;
                exponent = true;
                is_float = true;

                type = NumberType::DecimalFloat;

                // exponent sign
                if (current != end && (*current == '+' || *current == '-')) {
                    value.push_back(*current);
                    current++;
                }
            }
            else if (!exponent && (*current == 'p' || *current == 'P')) {
                // hex exponent
                if (!hex) { break; }

                dot      = true;  // allow no more dots
                exponent = true;
                is_float = true;

                value.push_back(*current);
                current++;
                type = NumberType::HexFloat;

                // exponent sign
                if (current != end && (*current == '+' || *current == '-')) {
                    value.push_back(*current);
                    current++;
                }
            }
            else {
                // no special intermittent characters, we break
                break;
            }
        }
        else {
            // number is line end, we break
            break;
        }
    }

    char is_long = 0;
    // only allow long/unsigned suffix once
    bool is_unsigned = false;
    if (current != end) {
        // float suffixes
        if (type != NumberType::Octal && std::tolower(*current) == 'f') {
            value.push_back(*current);
            current++;
            switch (type) {
                case NumberType::Decimal:
                    type = NumberType::DecimalFloat;
                    break;
                case NumberType::Hex:
                    type = NumberType::HexFloat;
                    break;
                default:
                    break;
            }
            is_float = true;
        } else if (is_float && std::tolower(*current) == 'l') {
            value.push_back(*current);
            is_long = true;
            current++;
        }
        else if (!is_float) {

            // order does not matter, so we loop this twice
            for (int i = 0; i < 2; i++) {
                if (!is_long && (std::tolower(*current) == 'l')) {
                    // long suffix
                    is_long = 1;

                    char _current = *current;
                    value.push_back(*current);
                    current++;

                    // long long (has to be same char)
                    if (current != end) {
                        if (*current == _current) {
                            value.push_back(*current);
                            current++;
                            is_long = 2;
                        }
                    }
                }

                // unsigned
                if (!is_unsigned && (std::tolower(*current) == 'u')) {
                    is_unsigned = true;
                    value.push_back(*current);
                    current++;
                }
            }
        }
    }


    if (is_float) {
        double val = std::stod(value);
        auto fval = (float)val;
        if (is_long || ((double)fval != val)) {
            return std::make_shared<NumericalConstant<double>>(TokenType::ConstDouble, val);
        }
        return std::make_shared<NumericalConstant<float>>(TokenType::ConstFloat, fval);
    }
    else {
        if (is_unsigned) {
            unsigned long long val = std::stoull(value);
            if (is_long == 2 || val >= ULONG_MAX) {
                return std::make_shared<NumericalConstant<unsigned long long>>(TokenType::ConstUnsignedLongLong, val);
            }
            else if (is_long == 1 || val >= UINT_MAX) {
                return std::make_shared<NumericalConstant<unsigned long>>(TokenType::ConstUnsignedLong, val);
            }
            else {
                return std::make_shared<NumericalConstant<unsigned int>>(TokenType::ConstUnsignedInt, val);
            }
        }
        else {
            long long val = std::stoll(value);
            if (is_long == 2 || val >= LONG_MAX) {
                return std::make_shared<NumericalConstant<long long>>(TokenType::ConstLongLong, val);
            }
            else if (is_long == 1 || val >= INT_MAX) {
                return std::make_shared<NumericalConstant<long>>(TokenType::ConstLong, val);
            }
            else {
                return std::make_shared<NumericalConstant<int>>(TokenType::ConstInt, val);
            }
        }
    }
}