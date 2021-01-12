#include "tokenizer.h"

#include "punctuators.h"
#include "keywords.h"

#include "variant_util.h"
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
            if (*current == 'L' || std::tolower(*current) == 'u') {
                // might be character sequence
                if (next != line.end()) {
                    if (*next == '\"' || (*next == '8' && next + 1 != line.end() && *(next + 1) == '\"')) {
                        // string literal with encoding
                        ReadCharStringConstant(current, line.end(), current_token, '\"');
                        Tokens.emplace_back(StringConstant(current_token));
                        log_debug("Prefixed string literal: %s", current_token.c_str());
                        continue;
                    }
                    else if (*next == '\'') {
                        // char literal with encoding
                        ReadCharStringConstant(current, line.end(), current_token, '\"');
                        Tokens.emplace_back(CharStringConstant(current_token));
                        log_debug("Prefixed char literal: %s", current_token.c_str());
                        continue;
                    }
                }
            }

            for (; (current != line.end()) && (std::isalnum(*current) || *current == '_'); current++) {
                current_token.push_back(*current);
            }

            if (Keywords.find(current_token) != Keywords.end()) {
                Tokens.emplace_back(Keywords.at(current_token));
                log_debug("Keyword: %s", current_token.c_str());
            }
            else {
                Tokens.emplace_back(Identifier(current_token));
                log_debug("Identifier: %s", current_token.c_str());
            }
        }
        else if (std::isdigit(*current)) {
numeric_constant:
            auto constant = ReadNumericConstant(current, line.end(), current_token);
            Tokens.emplace_back(variant_cast(constant));
            log_debug("Numerical constant: %s", current_token.c_str());
        }
        else if (std::isspace(*current)) {
            // skip whitespace
            for (; (current != line.end()) && (std::isspace(*current)); current++) {}
        }
        else {
            if (*current == '"') {
                // string literal
                ReadCharStringConstant(current, line.end(), current_token, '\"');
                Tokens.emplace_back(StringConstant(current_token));
                log_debug("String literal: %s", current_token.c_str());
            }
            else if (*current == '\'') {
                // char literal
                ReadCharStringConstant(current, line.end(), current_token, '\'');
                Tokens.emplace_back(CharStringConstant(current_token));
                log_debug("Char literal: %s", current_token.c_str());
            }
            else {
                // punctuator
                if (*current == '.' && current != line.end() && std::isdigit(*next)) {
                    goto numeric_constant;
                }

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