#include <iostream>

#include "file/File.h"
#include "file/SString.h"
#include "tokenizer/Tokenizer.h"
#include "types/Types.h"
#include "parser/Parser.h"
#include "taxy/Declaration.h"
#include "taxy/Statement.h"
#include "phyte/Emitter.h"
#include "calyx/backend/interpreter/Interpreter.h"


#include "Log.h"


int main() {
  auto file = epi::File("tests/test.c");
  auto string = epi::SString(
          "{ for (int i = {0}; i < 12; i++)"
          "{ int a = i + 1, (*b)() = c = 12 ? 1 > 2 : 3; (&a)[i] = 69; }"
          "{ typedef int i32; *(i32*)abc = (int){0, 1, .abc = 2, [0].x[1] = 3}; }"
          "{ const struct TestStruct{ int a, b: 2, *c;} x = {1, 2, {3}}; }"
          " enum a {x = 1, y, z }; "
          " return x * 12 * 12;}");
  auto tokenizer = epi::Tokenizer(file);
  auto parser = epi::Parser(tokenizer);

#if 0
#define try if (true)
#define catch_e ; for (std::runtime_error e(""); false;)
#else
#define catch_e catch (std::runtime_error& e)
#endif

  try {
    parser.Parse();
    parser.Data();
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Parser error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
    file.PrintLoc();
  }

  auto emitter = epi::phyte::Emitter();
  try {
    emitter.MakeProgram(parser.declarations);
    std::cout << std::endl << std::endl;
    std::cout << "-- program" << std::endl;
    for (const auto& op : emitter.program) {
      std::cout << op->ToString() << std::endl;
    }
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Emitter error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
  }

  try {
    auto interpreter = epi::calyx::Interpreter();
    std::cout << std::endl << std::endl;
    std::cout << "-- interpreted" << std::endl;
    interpreter.EmitProgram(emitter.program);
    std::cout << std::endl << std::endl;
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Interpreter error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
  }

  return 0;
}
