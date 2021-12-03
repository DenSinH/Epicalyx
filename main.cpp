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
#include "cycle/Cycle.h"


#include "Log.h"


int main() {
  // auto file = epi::File("tests/test.c");
  auto file = epi::File("examples/emitting/switch.c");
  auto tokenizer = epi::Tokenizer(file);
  auto parser = epi::Parser(tokenizer);

#if 1
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
    for (int i = 0; i < emitter.program.size(); i++) {
      std::cout << 'L' << i << std::endl;
      for (const auto& op : emitter.program[i]) {
        std::cout << "    " << op->ToString() << std::endl;
      }
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
    interpreter.VisualizeProgram(emitter.program);
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Interpreter error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
  }

  return 0;
}
