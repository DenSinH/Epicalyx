#include <cstdio>

#include <tokenizer.h>
#include <parser.h>
#include <declaration_nodes.h>


int main() {
    auto t = new Tokenizer();

    t->Tokenize("examples/parsing/declarations/static_assert.decl");

    auto p = new Parser(t);

    auto n = p->ExpectStaticAssert();
    n->Print();

    return 0;
}
