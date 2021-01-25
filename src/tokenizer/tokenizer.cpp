#include "tokenizer.h"

#include "punctuators.h"
#include "keywords.h"

#include "log.h"

#include <fstream>
#include <cstdio>
#include <stdexcept>


void Tokenizer::Tokenize(std::shared_ptr<const File> file) {
    const auto ctx = context("Tokenizing file: " + *file->FileName);
    try {
        FileObj = file;
        LineNo = 1;
        for (auto& line : file->Lines) {
            Line = line;
            TokenizeLine(*line);
            LineNo++;
        }
    }
    catch (std::runtime_error& e) {
        std::string message;
        message = e.what();
        message += "\n" + Trace();
        // add fake token
        Tokens.push_back(MakeToken<Token>(TokenClass::StringConstant, TokenType::ConstString));
        message += "\n" + Tokens.back()->Loc();
        log_fatal("%s", message.c_str());
    }
}

void Tokenizer::TokenizeLine(const std::string& line) {
    for (auto current = line.begin(); current != line.end();) {
        current_token = "";
        auto next = current + 1;

        if (std::isalpha(*current) || (*current == '_')) {
            // identifier or keyword
            if (*current == 'L' || std::tolower(*current) == 'u') {
                // might be character sequence
                if (next != line.end()) {
                    if (*next == '\"' || (*next == '8' && next + 1 != line.end() && *(next + 1) == '\"')) {
                        // string literal with encoding
                        auto value = ReadStringConstant(current, line.end());
                        Tokens.emplace_back(value);
                        log_tokenizer("Prefixed string literal: %s", value->Repr().c_str());
                        continue;
                    }
                    else if (*next == '\'') {
                        // char literal with encoding
                        // char strings are actually constant int's
                        auto value = ReadCharSequenceConstant(current, line.end());
                        Tokens.emplace_back(value);
                        log_tokenizer("Prefixed char literal: %s", value->Repr().c_str());
                        continue;
                    }
                }
            }

            for (; (current != line.end()) && (std::isalnum(*current) || *current == '_'); current++) {
                current_token.push_back(*current);
            }

            if (Keywords.find(current_token) != Keywords.end()) {
                Tokens.emplace_back(MakeToken<Token>(TokenClass::Keyword, Keywords.at(current_token)));
                log_tokenizer("Keyword: %s", current_token.c_str());
            }
            else {
                Tokens.emplace_back(MakeToken<Identifier>(current_token));
                log_tokenizer("Identifier: %s", current_token.c_str());
            }
        }
        else if (std::isdigit(*current)) {
numeric_constant:
            auto constant = ReadNumericConstant(current, line.end());
            Tokens.emplace_back(constant);
            log_tokenizer("%s", constant->Repr().c_str());
        }
        else if (std::isspace(*current)) {
            // skip whitespace
            for (; (current != line.end()) && (std::isspace(*current)); current++) {}
        }
        else {
            if (*current == '"') {
                // string literal
                auto value = ReadStringConstant(current, line.end());
                Tokens.emplace_back(value);
                log_tokenizer("String literal: %s", value->Repr().c_str());
            }
            else if (*current == '\'') {
                // char literal
                auto value = ReadCharSequenceConstant(current, line.end());
                Tokens.emplace_back(value);
                log_tokenizer("Char literal: %s", value->Repr().c_str());
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
                    throw std::runtime_error("Invalid token: " + std::to_string(*current));
                }
                log_tokenizer("Punctuator: %s", current_token.c_str());
                Tokens.emplace_back(MakeToken<Token>(TokenClass::Punctuator, Punctuators.at(current_token)));
            }
        }
    }
}