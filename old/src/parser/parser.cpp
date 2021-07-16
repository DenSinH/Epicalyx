#include "parser.h"


std::vector<NODE(Node)> Parser::Parse() {
    std::vector<NODE(Node)> program = {};
    while (!EndOfStream()) {
        program.push_back(ExpectTranslationUnit());
    }
    return program;
}