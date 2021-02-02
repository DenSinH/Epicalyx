#include <cstdio>

#include <tokenizer.h>
#include <parser.h>


int main() {
    auto file = std::make_shared<File>("examples/parsing/program/typedefs.c");

    auto t = new Tokenizer();

    t->Tokenize(file);

//    auto tok = MAKE_TOKEN(Token)(std::make_shared<const std::string>(), 0, std::make_shared<const std::string>(), TokenClass::Identifier, TokenType::Identifier);
//    auto test = MAKE_NODE(PrimaryExpressionIdentifier)(tok, "test");
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
