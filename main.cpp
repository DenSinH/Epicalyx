#include <cstdio>

#include <tokenizer.h>


int main() {

    auto t = new Tokenizer();

    t->Tokenize("examples/numerical_constants.c");

    return 0;
}
