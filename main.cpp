#include <cstdio>

#include <tokenizer.h>
#include <parser.h>


int main() {

    auto t = new Tokenizer();

    t->Tokenize("examples/parsing/primary/identifier.expr");

    auto p = new Parser(t);

    auto n = std::move(p->ExpectPrimaryExpression());

    n->Print();

    return 0;
}
