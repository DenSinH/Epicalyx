#include "tokenizer.h"

#include "punctuators.h"
#include "keywords.h"

#include "log.h"

#include <fstream>
#include <cstdio>
#include <stdexcept>


void Tokenizer::Tokenize(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file.good()) {
        log_fatal("Could not open file!");
    }

    std::string line;
    while (std::getline(file, line)) {
        TokenizeLine(file, line);
    }
}

void Tokenizer::TokenizeLine(std::ifstream& file, const std::string& line) {
    std::string current_token;
    printf("%s\n", line.c_str());

    for (auto current = line.begin(); current != line.end();) {
        current_token = "";
        auto next = current + 1;

        if (std::isalpha(*current)) {
            // identifier or keyword
            for (; (current != line.end()) && (std::isalnum(*current) || *current == '_'); current++) {
                current_token.push_back(*current);
            }

            if (Keywords.find(current_token) != Keywords.end()) {
                log_debug("Keyword: %s", current_token.c_str());
            }
            else {
                log_debug("Identifier: %s", current_token.c_str());
            }
        }
        else if (std::isdigit(*current)) {
            ReadNumericConstant(current, line.end(), current_token);
            log_debug("Numerical constant: %s", current_token.c_str());
        }
        else if (std::isspace(*current)) {
            // skip whitespace
            for (; (current != line.end()) && (std::isspace(*current)); current++) {}
        }
        else {
            // todo: . might be start of a float

            if (*current == '"') {
                // string literal
                current++;
            }
            else if (*current == '\'') {
                // char literal
                current++;
            }
            else {
                // punctuator
                do {
                    // keep appending characters until the current token is no longer a punctuator
                    // or until we have reached the end of the line
                    current_token.push_back(*current);
                    current++;
                } while (current != line.end()
                         && !std::isalnum(*current)
                         && Punctuators.find(current_token) != Punctuators.end());

                if (Punctuators.find(current_token) == Punctuators.end()) {
                    // loop ended because our current_token is no longer a punctuator, so we must step back one character
                    current--;
                    current_token.pop_back();
                }

                if (!current_token.length()) {
                    log_fatal("Invalid token: %c", *current);
                }
                log_debug("Punctuator: %s", current_token.c_str());
                Tokens.emplace_back(Punctuators.at(current_token));
            }
        }
    }
}

void Tokenizer::ReadNumericConstant(std::string::const_iterator& current, std::string::const_iterator end, std::string& dest) {
    NumberType type = NumberType::Decimal;
    bool dot = false;
    bool exponent = false;
    bool hex = false;
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

                dest.push_back('e');
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

                dest.push_back('p');
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