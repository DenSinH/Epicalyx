#include "tokenizer.h"


enum class NumberType {
    Decimal,
    Octal,
    Hex,
    DecimalFloat,
    HexFloat,
};

void Tokenizer::ReadNumericConstant(std::string::const_iterator& current, std::string::const_iterator end, std::string& dest) {
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

    if (current != end) {
        if (is_float) {
            // float suffixes
            if (std::tolower(*current) == 'f') {
                dest.push_back(*current);
                current++;
            } else if (std::tolower(*current) == 'l') {
                dest.push_back(*current);
                current++;
            }
        }
        else {
            // only allow long/unsigned suffix once
            bool is_long     = false;
            bool is_unsigned = false;

            // order does not matter, so we loop this twice
            for (int i = 0; i < 2; i++) {
                if (!is_long && (std::tolower(*current) == 'l')) {
                    // long suffix
                    is_long = true;

                    char _current = *current;
                    dest.push_back(*current);
                    current++;

                    // long long (has to be same char)
                    if (current != end) {
                        if (*current == _current) {
                            dest.push_back(*current);
                            current++;
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
}