#include <cstdio>

#include <tokenizer.h>
#include <parser.h>
#include <declaration_nodes.h>


int main() {
    auto t = new Tokenizer();

    t->Tokenize("examples/parsing/declarations/struct.decl");

    auto p = new Parser(t);

    auto n = p->ExpectStructUnionSpecifier();
    n->Print();

    return 0;
}
