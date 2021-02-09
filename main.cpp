#include <tokenizer.h>
#include <parser.h>

#include <functional>


int main() {
    auto file = std::make_shared<File>("examples/parsing/program/typedefs.c");

    auto t = new Tokenizer();

    CTYPE Int = MAKE_TYPE(ValueType<i32>)(1, CType::LValueNess::Assignable, 0);
    CTYPE UnknownInt = MAKE_TYPE(ValueType<i32>)(CType::LValueNess::None, 0);
    CTYPE Float = MAKE_TYPE(ValueType<float>)(1.2, CType::LValueNess::None, 0);
    CTYPE FloatPtr = MAKE_TYPE(PointerType)(
            Float, CType::LValueNess::None
    );
    CTYPE IntPtr = MAKE_TYPE(PointerType)(Int, CType::LValueNess::None);
    CTYPE IntArray = MAKE_TYPE(ArrayType)(Int, Int);
    CTYPE FuncPtr = MAKE_TYPE(FunctionType)(Int, "my_function");

    auto shifted = (*Int).RShift(*Int);
    auto equal = (*UnknownInt).Eq(*Int);
    auto notequal = (*Int).Neq(*Int);
    auto ptr_sub = (*IntPtr).Sub(*IntPtr);
    auto array_add = (*Int).Add(*IntArray);
    auto valid_ref = (*Int).Ref();
    // auto invalid_ref = (*Float).Ref();  // not lvalue, throws exception
    // auto invalid = (*Float).Mod(*Int);  // throws exception

    std::cout << IntPtr->ToString() << std::endl;
    std::cout << IntArray->EqualType(*IntPtr) << std::endl;
    std::cout << valid_ref->ToString() << std::endl;
    std::cout << shifted->ToString() << std::endl;
    std::cout << equal->ToString() << std::endl;
    std::cout << notequal->ToString() << std::endl;
    std::cout << ptr_sub->ToString() << std::endl;
    std::cout << array_add->ToString() << std::endl;
    std::cout << IntPtr->Deref()->ToString() << std::endl;
    std::cout << IntPtr->Deref()->Ref()->Deref()->Ref()->Deref()->Ref()->Deref()->ToString() << std::endl;
    std::cout << FuncPtr->Deref()->Deref()->Deref()->Deref()->Deref()->Deref()->ToString() << std::endl;

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
