#include <iostream>

#include "ir_emitter/Emitter.h"
#include "calyx/backend/interpreter/Interpreter.h"
#include "optimizer/ProgramDependencies.h"
#include "optimizer/BasicOptimizer.h"
#include "tokenizer/Preprocessor.h"
#include "tokenizer/Tokenizer.h"
#include "parser/Parser.h"
#include "regalloc/RIG.h"
#include "regalloc/regspaces/Example.h"


#include "Log.h"


#undef catch_errors

#ifndef catch_errors
#define try if (true)
#define catch_e ; for (std::runtime_error e(""); false;)
#else
#define catch_e catch (std::runtime_error& e)
#endif


int main() {
  std::string file = "examples/emitting/branches.c";
  std::string rig_func_sym = "test";
  auto preprocessor = epi::Preprocessor(file);
  auto tokenizer = epi::Tokenizer(preprocessor);
  auto parser = epi::Parser(tokenizer);

  try {
    parser.Parse();
    parser.Data();
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Parser error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
    preprocessor.PrintLoc();
    return 1;
  }

  auto emitter = epi::Emitter();
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
  epi::calyx::PrintProgram(program);

  for (auto& [sym, func] : program.functions) {
    // repeating multiple times will link more blocks
    auto func_hash = func.Hash();
    while (true) {
      std::cout << "Optimizing program hash " << func_hash << "..." << std::endl;
      try {
        auto optimizer = epi::BasicOptimizer(std::move(func));
        func = optimizer.Optimize();

        auto new_hash = func.Hash();
        if (func_hash == new_hash) break;
        func_hash = new_hash;
      }
      catch_e {
        Log::ConsoleColor<Log::Color::Red>();
        std::cout << "Optimizer error:" << std::endl << std::endl;
        std::cout << e.what() << std::endl;
        return 1;
      }
    }
  }

  epi::calyx::PrintProgram(program);

  std::cout << std::endl << std::endl;
  std::cout << "-- globals" << std::endl;
  for (const auto& [symbol, global] : program.globals) {
    std::cout << symbol << " ";
    std::visit([](auto& glob) {
      using glob_t = std::decay_t<decltype(glob)>;
      if constexpr(std::is_same_v<glob_t, epi::calyx::Pointer>) {
        std::cout << "%p" << std::hex << glob.value << std::endl;
      }
      else if constexpr(std::is_same_v<glob_t, epi::calyx::label_offset_t>) {
        if (glob.offset) {
          std::cout << glob.label << "+" << glob.offset << std::endl;
        }
        else {
          std::cout << glob.label << std::endl;
        }
      }
      else {
        std::cout << glob << std::endl;
      }
    }, global);
  }

  for (const auto& string : program.strings) {
    std::cout << '"' << string << '"' << std::endl;
  }

  epi::calyx::VisualizeProgram(program, "output/program.pdf");

  try {
    auto interpreter = epi::calyx::Interpreter(program);
    std::cout << std::endl << std::endl;
    std::cout << "-- interpreted" << std::endl;
    interpreter.EmitProgram(program);

    // extract globals from interpreter
    std::cout << "  -- globals" << std::endl;
    for (const auto& [glob, glob_idx] : interpreter.globals) {
      std::cout << "  " << glob << " ";
      std::visit([&](auto& pglob) {
        using glob_t = std::decay_t<decltype(pglob)>;
        glob_t data;
        std::memcpy(&data, interpreter.global_data[glob_idx].data(), sizeof(glob_t));
        if constexpr(std::is_same_v<glob_t, epi::calyx::Pointer>) {
          std::cout << "%p" << std::hex << data.value << std::endl;
        }
        else if constexpr(std::is_same_v<glob_t, epi::calyx::label_offset_t>) {
          if (data.offset) {
            std::cout << data.label << "+" << data.offset << std::endl;
          }
          else {
            std::cout << data.label << std::endl;
          }
        }
        else {
          std::cout << data << std::endl;
        }
      }, program.globals.at(glob));
    }

    auto dependencies = epi::ProgramDependencies::GetDependencies(program);
    dependencies.VisualizeVars("output/vars.pdf");
  }
  catch_e {
    Log::ConsoleColor<Log::Color::Red>();
    std::cout << "Interpreter error:" << std::endl << std::endl;
    std::cout << e.what() << std::endl;
    return 1;
  }

  if (!program.functions.contains(rig_func_sym)) {
    rig_func_sym = "main";
  }
  const auto& rig_func = program.functions.at(rig_func_sym);
  auto rig = epi::RIG::GenerateRIG(rig_func);

  auto regspace = epi::RegisterSpace::GetRegSpace<epi::ExampleRegSpace>(rig_func);
  for (const auto& [gvar, regtype] : regspace->register_type_map) {
    if (gvar.is_local) std::cout << 'c';
    else std::cout << 'v';
    std::cout << gvar.idx << ' ';
    switch (regtype) {
      case epi::ExampleRegSpace::RegType::GPR: std::cout << "GPR"; break;
      case epi::ExampleRegSpace::RegType::FPR: std::cout << "FPR"; break;
    }
    std::cout << std::endl;
  }

  rig.Reduce(*regspace);
  rig.Visualize("output/rig.pdf");

  return 0;
}
