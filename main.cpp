#include <cstdio>

#include <tokenizer.h>
#include <parser.h>


int main() {

    auto t = new Tokenizer();

    t->Tokenize("examples/parsing/expression/combined.expr");

    auto p = new Parser(t);

//    try {
        auto n = std::move(p->ExpectExpression());
        n->Print();
//    }
//    catch (const std::exception& exc) {
//        log_fatal("%s", exc.what());
//    }

    return 0;
}
