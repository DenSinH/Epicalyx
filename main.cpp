#include <cstdio>

#include <tokenizer.h>
#include <parser.h>


int main() {

    auto t = new Tokenizer();

    t->Tokenize("examples/parsing/binary/combined.expr");

    auto p = new Parser(t);

    auto n = std::move(p->ExpectLogicOrExpression());

    n->Print();

    return 0;
}
