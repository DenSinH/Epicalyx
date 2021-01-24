#include <cstdio>

#include <tokenizer.h>
#include <parser.h>
#include <declaration_nodes.h>


int main() {
    auto t = new Tokenizer();

    t->Tokenize("examples/parsing/expressions/postfix/type_initializer.expr");

    auto p = new Parser(t);

    auto n = p->ExpectExpression();
    n->Print();

    return 0;
}
