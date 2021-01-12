#include "tokenizer.h"
#include <climits>

enum class NumberType {
    Decimal,
    Octal,
    Hex,
    DecimalFloat,
    HexFloat,
};

std::variant<NumericalConstant<double>, NumericalConstant<unsigned long long>> Tokenizer::ReadNumericConstant(
        std::string::const_iterator& current,
        std::string::const_iterator end,
        std::string& dest
) {
    NumberType type = NumberType::Decimal;
    bool dot      = false;
    bool exponent = false;
    bool hex      = false;
    bool is_float = false;

    auto next = current + 1;
    // we already know that we are starting with a digit
    dest.push_back(*current);

    if (*current == '0') {
        if ((current != end) && (*next == 'x' || *next == 'X')) {
            // hex-prefex
            type = NumberType::Hex;
            dest.push_back('x');
            hex = true;
            current++;
        }
        else {
            // octal prefix
            type = NumberType::Octal;
        }
    }
    else if (*current == '.') {
        dest.push_back(*current);
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
            dest.push_back(*current);
            current++;
        }

        // we might have a dot for a float / double
        if (current != end) {
            if (!dot && (*current == '.')) {
                // (first) dot, number is a decimal float, other types of floats may not contain dots
                if (hex) { break; } // dots not allowed in hex float

                dest.push_back(*current);
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

                dest.push_back(*current);
                current++;

                // don't allow more dots
                dot      = true;
                exponent = true;
                is_float = true;

                type = NumberType::DecimalFloat;

                // exponent sign
                if (current != end && (*current == '+' || *current == '-')) {
                    dest.push_back(*current);
                    current++;
                }
            }
            else if (!exponent && (*current == 'p' || *current == 'P')) {
                // hex exponent
                if (!hex) { break; }

                dot      = true;  // allow no more dots
                exponent = true;
                is_float = true;

                dest.push_back(*current);
                current++;
                type = NumberType::HexFloat;

                // exponent sign
                if (current != end && (*current == '+' || *current == '-')) {
                    dest.push_back(*current);
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
            dest.push_back(*current);
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
            dest.push_back(*current);
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
                    dest.push_back(*current);
                    current++;

                    // long long (has to be same char)
                    if (current != end) {
                        if (*current == _current) {
                            dest.push_back(*current);
                            current++;
                            is_long = 2;
                        }
                    }
                }

                // unsigned
                if (!is_unsigned && (std::tolower(*current) == 'u')) {
                    is_unsigned = true;
                    dest.push_back(*current);
                    current++;
                }
            }
        }
    }


    if (is_float) {
        if (is_long) {
            return NumericalConstant<double>(TokenType::ConstDouble, std::stod(dest));
        }
        return NumericalConstant<double>(TokenType::ConstFloat, std::stof(dest));
    }
    else {
        if (is_unsigned) {
            unsigned long long value = std::stoull(dest);
            if (is_long == 2 || value >= ULONG_MAX) {
                return NumericalConstant<unsigned long long>(TokenType::ConstUnsignedLongLong, value);
            }
            else if (is_long == 1 || value >= UINT_MAX) {
                return NumericalConstant<unsigned long long>(TokenType::ConstUnsignedLong, value);
            }
            else {
                return NumericalConstant<unsigned long long>(TokenType::ConstUnsignedInt, value);
            }
        }
        else {
            long long value = std::stoll(dest);
            if (is_long == 2 || value >= LONG_MAX) {
                return NumericalConstant<unsigned long long>(TokenType::ConstLongLong, value);
            }
            else if (is_long == 1 || value >= INT_MAX) {
                return NumericalConstant<unsigned long long>(TokenType::ConstLong, value);
            }
            else {
                return NumericalConstant<unsigned long long>(TokenType::ConstInt, value);
            }
        }
    }
}