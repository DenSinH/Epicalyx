#include <cstdio>

#include <tokenizer.h>
#include <parser.h>


int main() {

    auto t = new Tokenizer();

    t->Tokenize("examples/parsing/unary/combined.expr");

    auto p = new Parser(t);

    auto n = std::move(p->ExpectUnaryExpression());

    n->Print();

    return 0;
}
