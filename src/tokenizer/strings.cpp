#include "tokenizer.h"

#include "log.h"

#include <stdexcept>

static constexpr int ASCIIHexToInt[] =
{
    // ASCII
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};


TOKEN Tokenizer::ReadStringConstant(std::string::const_iterator& current, const std::string::const_iterator& end) {
    auto ctx = context("scanning string literal");

    current_token = "";
    // string encoding
    if (*current == 'L' || *current == 'U') {
        current++;
    }
    else if (*current == 'u') {
        current++;
        if (current != end && *current == '8') {
            current++;
        }
    }

    if (*current != '"') {
        throw std::runtime_error("Invalid string constant read started: " + std::to_string(*current));
    }
    current++;
    while (current != end && *current != '"') {
        current_token.push_back(ReadCChar(current, end));
    }

    if (current == end) {
        throw std::runtime_error("End of line while scanning string literal");
    }
    else if (*current != '"') {
        throw std::runtime_error("Error while scanning string literal: ended on '"+ std::to_string(*current) +"' instead of \"");
    }
    // skip last char
    current++;
    return MakeToken<StringConstant>(current_token);
}

TOKEN Tokenizer::ReadCharSequenceConstant(std::string::const_iterator& current, const std::string::const_iterator& end) {
    auto ctx = context("scanning char literal");

    bool is_long = false;
    bool is_unsigned = false;
    if (std::tolower(*current) == 'l') {
        // char encoding
        is_long = true;
        current++;
    } else if (std::tolower(*current) == 'u') {
        is_unsigned = true;
        current++;
    }

    if (*current != '\'') {
        throw std::runtime_error("Invalid char sequence constant read started: " + std::to_string(*current));
    }
    current++;
    unsigned long long value = 0;
    while (current != end && *current != '\'') {
        value <<= 8;
        value |= ReadCChar(current, end);
    }

    if (current == end) {
        throw std::runtime_error("End of line while scanning string literal");
    }
    else if (*current != '\'') {
        throw std::runtime_error("Error while scanning char sequence literal: ended on '" + std::to_string(*current) + "' instead of '");
    }
    // skip last char
    current++;

    if (is_unsigned) {
        return MakeToken<NumericalConstant<unsigned long long>>(TokenType::ConstUnsignedInt, value);
    }
    else if (is_long) {
        return MakeToken<NumericalConstant<long long>>(TokenType::ConstLongLong, value);
    }
    else {
        return MakeToken<NumericalConstant<int>>(TokenType::ConstInt, value);
    }
}

unsigned char Tokenizer::ReadCChar(std::string::const_iterator& current, const std::string::const_iterator& end) {
    unsigned char value;
    if (*current == '\\') {
        // escape sequence
        current++;
        if (current == end) {
            throw std::runtime_error("End of line while scanning escape sequence");
        }

        switch (*current) {
            case '\'':
            case '"':
            case '\\':
                value = *current;
                break;
            case 'a':
                value = '\a';
                break;
            case 'b':
                value = '\b';
                break;
            case 'f':
                value = '\f';
                break;
            case 'n':
                value = '\n';
                break;
            case 'r':
                value = '\r';
                break;
            case 't':
                value = '\t';
                break;
            case 'v':
                value = '\v';
                break;
            case '?':
                value = '\?';
                break;
            case 'x':
            {
                current++;
                if (current == end) {
                    log_fatal("End of line while scanning escape sequence");
                }

                if (!std::isxdigit(*current)) {
                    log_fatal("Invalid hex char escape sequence");
                }
                value = ASCIIHexToInt[*current];
                current++;
                if (current != end && std::isxdigit(*current)) {
                    // double hex digit char escape
                    value <<= 4;
                    value |= ASCIIHexToInt[*current];
                }
                else {
                    // overscan
                    current--;
                }
                break;
            }
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
            {
                value = (*current) - '0';
                current++;
                if (current != end && std::isdigit(*current) && *current < '8') {
                    // double oct digit escape
                    value <<= 3;
                    value |= (*current) - '0';
                    current++;
                    if (current != end && std::isdigit(*current) && *current < '8') {
                        // triple oct digit escape
                        value <<= 3;
                        value |= (*current) - '0';
                        current++;
                    }
                }
                // we always go one too far
                current--;
                break;
            }
            default:
                throw std::runtime_error("Invalid escape sequence: '" + std::to_string(*current) + "'");
        }
        current++;
    }
    else {
        value = *current;
        current++;
    }
    return value;
}