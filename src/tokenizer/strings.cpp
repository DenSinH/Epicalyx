#include "tokenizer.h"

#include "log.h"

static constexpr int ASCIIHexToInt[] =
{
    // ASCII
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};


void Tokenizer::ReadCharStringConstant(std::string::const_iterator& current, std::string::const_iterator end, std::string& dest, const char terminator) {
    // terminator is either ' or ""
    if (terminator == '\'' && std::tolower(*current) == 'l' || std::tolower(*current) == 'u') {
        // char encoding
        current++;
    }
    else if (terminator == '"') {
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
    }

    if (*current != terminator) {
        log_fatal("Invalid char constant read started: %c", *current);
    }
    current++;
    while (current != end && *current != terminator) {
        if (*current == '\\') {
            // escape sequence
            current++;
            if (current == end) {
                log_fatal("End of line while scanning escape sequence");
            }

            switch (*current) {
                case '\'':
                case '"':
                case '\\':
                    dest.push_back(*current);
                    break;
                case 'a':
                    dest.push_back('\a');
                    break;
                case 'b':
                    dest.push_back('\b');
                    break;
                case 'f':
                    dest.push_back('\f');
                    break;
                case 'n':
                    dest.push_back('\n');
                    break;
                case 'r':
                    dest.push_back('\r');
                    break;
                case 't':
                    dest.push_back('\t');
                    break;
                case 'v':
                    dest.push_back('\v');
                    break;
                case '?':
                    dest.push_back('\?');
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
                    unsigned char hex = ASCIIHexToInt[*current];
                    if (current != end && std::isxdigit(*current)) {
                        current++;
                        // double hex digit char escape
                        hex <<= 4;
                        hex |= ASCIIHexToInt[*current];
                    }
                    dest.push_back(hex);
                    break;
                }
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                {
                    unsigned char oct = (*current) - '0';
                    current++;
                    if (current != end && std::isdigit(*current) && *current < '8') {
                        // double oct digit escape
                        oct <<= 3;
                        oct |= (*current) - '0';
                        current++;
                        if (current != end && std::isdigit(*current) && *current < '8') {
                            // triple oct digit escape
                            oct <<= 3;
                            oct |= (*current) - '0';
                            current++;
                        }
                    }
                    // we always go one too far
                    current--;
                    dest.push_back(oct);
                    break;
                }
                default:
                    log_fatal("Invalid escape sequence: '%c'", *current);
            }
            current++;
        }
        else {
            dest.push_back(*current);
            current++;
        }
    }

    if (current == end) {
        log_fatal("End of line while scanning string literal");
    }
    else if (*current != terminator) {
        log_fatal("Error while scanning string literal: ended on '%c' instead of %c", *current, terminator);
    }
    // skip last char
    current++;
}