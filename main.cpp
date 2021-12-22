#include <iostream>

#include "phyte/Emitter.h"
#include "calyx/backend/interpreter/Interpreter.h"
#include "optimizer/ProgramDependencies.h"
#include "optimizer/BasicOptimizer.h"
#include "optimizer/RemoveUnused.h"
#include "file/File.h"
#include "file/SString.h"
#include "tokenizer/Tokenizer.h"
#include "types/Types.h"
#include "parser/Parser.h"
#include "taxy/Declaration.h"
#include "taxy/Statement.h"


#include "Log.h"


int main() {
  // auto file = epi::File("tests/test.c");
  auto file = epi::File("examples/emitting/binop.c");
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
    return 1;
  }

  auto emitter = epi::phyte::Emitter();
  try {
    emitter.MakeProgram(parser.declarations);
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Emitter error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
    return 1;
  }

  auto program = std::move(emitter.program);

  try {
    auto optimizer = epi::BasicOptimizer();
    optimizer.EmitProgram(program);

    program = std::move(optimizer.new_program);
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Optimizer error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
    return 1;
  }

  try {
    auto cleanup = epi::RemoveUnused();
    cleanup.EmitProgram(program);

    program = std::move(cleanup.new_program);
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Cleanup error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
    return 1;
  }

  std::cout << std::endl << std::endl;
  std::cout << "-- program" << std::endl;
  for (int i = 0; i < program.blocks.size(); i++) {
    std::cout << 'L' << i << std::endl;
    for (const auto& op : program.blocks[i]) {
      std::cout << "    " << op->ToString() << std::endl;
    }
  }
  std::cout << std::endl << std::endl;
  std::cout << "-- globals" << std::endl;
  for (const auto& [symbol, size] : program.globals) {
    std::cout << symbol << " " << size << std::endl;
  }
  for (const auto& string : program.strings) {
    std::cout << '"' << string << '"' << std::endl;
  }

  try {
    auto interpreter = epi::calyx::Interpreter(program);
    std::cout << std::endl << std::endl;
    std::cout << "-- interpreted" << std::endl;
    interpreter.EmitProgram(program);
    std::cout << std::endl << std::endl;
    interpreter.VisualizeProgram(program);

    auto dependencies = epi::ProgramDependencies();
    dependencies.EmitProgram(program);
    dependencies.VisualizeVars();
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Interpreter error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
    return 1;
  }

  return 0;
}
