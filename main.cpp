#include <cstdio>

#include <tokenizer.h>
#include <parser.h>
#include <declaration_nodes.h>


int main() {
    auto file = std::make_shared<File>("examples/parsing/program/typedefs.c");

    auto t = new Tokenizer();

    t->Tokenize(file);

    auto p = new Parser(t);

//    try {
        auto program = p->Parse();
        for (auto& n : program) {
            n->Print();
        }
//    }
//    catch (std::runtime_error& e) {
//        std::string message = e.what();
//        if (!p->EndOfStream()) {
//            message += "\n" + p->Current()->Loc();
//        }
//        log_fatal("%s", message.c_str());
//    }

    return 0;
}
