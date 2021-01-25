#include <cstdio>

#include <tokenizer.h>
#include <parser.h>
#include <declaration_nodes.h>


int main() {
    auto file = std::make_shared<File>("examples/parsing/declarations/declaration.decl");

    auto t = new Tokenizer();

    t->Tokenize(file);

    auto p = new Parser(t);

    auto n = p->ExpectDeclaration();
    n->Print();

    return 0;
}
