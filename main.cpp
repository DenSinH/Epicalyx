#include <tokenizer.h>
#include <parser.h>

#include <functional>


int main() {
    auto file = std::make_shared<File>("examples/parsing/program/typedefs.c");

    auto t = new Tokenizer();

    CTYPE Int = MAKE_TYPE(ValueType<int>)(1, 0);
    CTYPE UnknownInt = MAKE_TYPE(ValueType<int>)(0);
    CTYPE FloatPtr = MAKE_TYPE(PointerType)(MAKE_TYPE(ValueType<float>)(1.2, 0));
    CTYPE IntPtr = MAKE_TYPE(PointerType)(Int);
    CTYPE IntArray = MAKE_TYPE(ArrayType)(Int, Int);

    auto shifted = (*Int).RShift(*Int);
    auto equal = (*UnknownInt).Eq(*Int);
    auto notequal = (*Int).Neq(*Int);
    auto ptr_sub = (*IntPtr).Sub(*IntPtr);
    auto array_add = (*Int).Add(*IntArray);

    std::cout << IntPtr->ToString() << std::endl;
    std::cout << IntArray->EqualType(*IntPtr) << std::endl;
    std::cout << shifted->ToString() << std::endl;
    std::cout << equal->ToString() << std::endl;
    std::cout << notequal->ToString() << std::endl;
    std::cout << ptr_sub->ToString() << std::endl;
    std::cout << array_add->ToString() << std::endl;

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
