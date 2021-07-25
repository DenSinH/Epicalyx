#include <iostream>

#include "file/File.h"
#include "file/SString.h"
#include "tokenizer/Tokenizer.h"
#include "types/Types.h"
#include "parser/Parser.h"

#include "nodes/Declaration.h"
#include "nodes/Statement.h"


int main() {
  auto file = epi::File("examples/parsing/program/typedefs.c");
  auto string = epi::SString(
          "{ for (int i = {0}; i < 12; i++)"
          "{ int a = i + 1, (*b)() = c = 12 ? 1 > 2 : 3; (&a)[i] = 69; }"
          "{ typedef int i32; *(i32*)abc = (int){0, 1, .abc = 2, [0].x[1] = 3}; }"
          "{ const struct TestStruct{ int a, b: 2, *c;} x = {1, 2, {3}}; }"
          " enum a {x = 1, y, z }; "
          " return x * 12 * 12;}");
  auto tokenizer = epi::Tokenizer(file);
  auto parser = epi::Parser(tokenizer);

#if 1
#define try if (true)
#define catch_e std::runtime_error e(""); if (false)
#else
#define catch_e catch (std::runtime_error& e)
#endif

  try {
    parser.Parse();
    parser.Data();
  }
  catch_e {
    std::cout << e.what() << std::endl;
//    string.PrintLoc();
  }

//  epi::pType<> Int = epi::MakeType<epi::ValueType<epi::i32>>(1, epi::CType::LValueNess::Assignable, 0);
//  epi::pType<> UnknownInt = epi::MakeType<epi::ValueType<epi::i32>>(epi::CType::LValueNess::None, 0);
//  epi::pType<> Float = epi::MakeType<epi::ValueType<float>>(1.2, epi::CType::LValueNess::None, 0);
//  epi::pType<> FloatPtr = epi::MakeType<epi::PointerType>(
//          Float, epi::CType::LValueNess::None
//  );
//  epi::pType<> IntPtr = epi::MakeType<epi::PointerType>(Int, epi::CType::LValueNess::None);
//  epi::pType<> IntArray = epi::MakeType<epi::ArrayType>(Int, Int);
//  epi::pType<> FuncPtr = epi::MakeType<epi::FunctionType>(Int, std::string{"my_function"});
//
//  auto shifted = (*Int).RShift(*Int);
//  auto equal = (*UnknownInt).Eq(*Int);
//  auto notequal = (*Int).Neq(*Int);
//  auto ptr_sub = (*IntPtr).Sub(*IntPtr);
//  auto array_add = (*Int).Add(*IntArray);
//  auto valid_ref = (*Int).Ref();
//// auto invalid_ref = (*Float).Ref();  // not lvalue, throws exception
//// auto invalid = (*Float).Mod(*Int);  // throws exception
//
//  std::cout << IntPtr->ToString() << std::endl;
//  std::cout << IntArray->EqualType(*IntPtr) << std::endl;
//  std::cout << valid_ref->ToString() << std::endl;
//  std::cout << shifted->ToString() << std::endl;
//  std::cout << equal->ToString() << std::endl;
//  std::cout << notequal->ToString() << std::endl;
//  std::cout << ptr_sub->ToString() << std::endl;
//  std::cout << array_add->ToString() << std::endl;
//  std::cout << IntPtr->Deref()->ToString() << std::endl;
//  std::cout << IntPtr->Deref()->Ref()->Deref()->Ref()->Deref()->Ref()->Deref()->ToString() << std::endl;
//  std::cout << FuncPtr->Deref()->Deref()->Deref()->Deref()->Deref()->Deref()->ToString() << std::endl;

  return 0;
}
