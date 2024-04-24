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
#include "config/Info.h"
#include "Decltype.h"


#include "Log.h"


#undef catch_errors

#ifndef catch_errors
#define try if (true)
#define catch_e ; for (std::runtime_error e(""); false;)
#else
#define catch_e catch (std::runtime_error& e)
#endif


int main(int argc, char** argv) {
  auto settings = epi::info::parse_args(argc, argv);
  auto rig_func_sym = epi::cotyl::CString(settings.rigfunc);

  std::unique_ptr<epi::Preprocessor> pp;

  try {
    pp = std::make_unique<epi::Preprocessor>(settings.filename);
  }
  catch_e {
    std::cerr << "Initialization error:" << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    std::exit(1);
  }

  auto tokenizer = epi::Tokenizer(*pp);
  auto parser = epi::Parser(tokenizer);

  try {
    parser.Parse();
    parser.Data();
  }
  catch_e {
    std::cerr << "Parser error:" << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    parser.PrintLoc();
    return 1;
  }

  auto emitter = epi::Emitter();
  try {
    emitter.MakeProgram(parser.declarations, parser.functions);
  }
  catch_e {
    std::cerr << "Emitter error:" << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
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
        std::cerr << "Optimizer error:" << std::endl << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
      }
    }
  }

  epi::calyx::PrintProgram(program);

  std::cout << std::endl << std::endl;
  std::cout << "-- globals" << std::endl;
  for (const auto& [symbol, global] : program.globals) {
    std::cout << symbol.c_str() << " ";
    std::visit([](auto& glob) {
      using glob_t = decltype_t(glob);
      if constexpr(std::is_same_v<glob_t, epi::calyx::Pointer>) {
        std::cout << "%p" << std::hex << glob.value << std::endl;
      }
      else if constexpr(std::is_same_v<glob_t, epi::label_offset_t>) {
        if (glob.offset) {
          std::cout << glob.label.c_str() << "+" << glob.offset << std::endl;
        }
        else {
          std::cout << glob.label.c_str() << std::endl;
        }
      }
      else {
        std::cout << glob << std::endl;
      }
    }, global);
  }

  for (const auto& string : program.strings) {
    std::cout << '"' << string.c_str() << '"' << std::endl;
  }

  epi::calyx::VisualizeProgram(program, "output/program.pdf");

  try {
    epi::calyx::Interpreter interpreter{};
    std::cout << std::endl << std::endl;
    std::cout << "-- interpreted" << std::endl;
    interpreter.Interpret(program);

    // extract globals from interpreter
    std::cout << "  -- globals" << std::endl;
    for (const auto& [glob, glob_idx] : interpreter.globals) {
      std::cout << "  " << glob.c_str() << " ";
      std::visit([&](auto& pglob) {
        using glob_t = decltype_t(pglob);
        glob_t data;
        std::memcpy(&data, interpreter.global_data[glob_idx].data(), sizeof(glob_t));
        if constexpr(std::is_same_v<glob_t, epi::calyx::Pointer>) {
          std::cout << "%p" << std::hex << data.value << std::endl;
        }
        else if constexpr(std::is_same_v<glob_t, epi::label_offset_t>) {
          if (data.offset) {
            std::cout << data.label.c_str() << "+" << data.offset << std::endl;
          }
          else {
            std::cout << data.label.c_str() << std::endl;
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
    std::cerr << "Interpreter error:" << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (!program.functions.contains(rig_func_sym)) {
    std::cout << "No function " << rig_func_sym.view()
              << " exists, defaulting to 'main'..." << std::endl;
    rig_func_sym = epi::cotyl::CString("main");
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
