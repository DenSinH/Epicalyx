#include "tokenizer.h"

#include <fstream>
#include <cstdio>

void Tokenizer::Tokenize(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file.good()) {
        throw std::exception("Could not open file!");
    }

    std::string line;
    while (std::getline(file, line)) {
        TokenizeLine(file, line);
    }
}

void Tokenizer::TokenizeLine(std::ifstream& file, const std::string& line) {
    std::string current_token = "";
    auto current = line.begin();

    for (auto current = line.begin(); current != line.end(); current++) {
        auto next = current + 1;
        if (std::isalpha(*current)) {
            // identifier or keyword
            for (; (current != line.end()) && (std::isalnum(*current) || *current == '_'); current++) {
                current_token.push_back(*current);
            }
        }
        else if (std::isspace(*current)) {
            // skip whitespace
            for (; (current != line.end()) && (std::isspace(*current)); current++) {}
        }
        else {
            // punctuator
        }

        printf("%c%c", *current, *next);
    }

    printf("\n");
}