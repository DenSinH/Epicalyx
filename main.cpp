#include <cstdio>

#include <tokenizer.h>


int main() {

    auto t = new Tokenizer();

    t->Tokenize("examples/hello_world.c");

    return 0;
}
