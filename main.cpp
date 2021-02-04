#include <cstdio>

#include <tokenizer.h>
#include <parser.h>

#include <functional>


int main() {
    auto file = std::make_shared<File>("examples/parsing/program/typedefs.c");

    auto t = new Tokenizer();

//    auto Int = MAKE_TYPE(ValueType<int>)(1, 0);
//    auto Ptr = MAKE_TYPE(PointerType)(MAKE_TYPE(ValueType<float>)(1.2, 0));
//    auto sum = TypePropagation::Add(Int, Ptr);
//
//    std::cout << sum->to_string() << std::endl;

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
