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
#include "Exceptions.h"


#include "Log.h"

using ::epi::stringify;

int parse_program(const epi::info::ProgramSettings& settings) {
  auto rig_func_sym = epi::cotyl::CString(settings.rigfunc);

  std::unique_ptr<epi::Preprocessor> pp;

  try {
    pp = std::make_unique<epi::Preprocessor>(settings.filename);
  }
  catch (epi::cotyl::Exception& e) {
    if (!settings.catch_errors) throw;
    std::cerr << e.title << ':' << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    std::exit(1);
  }

  auto tokenizer = epi::Tokenizer(*pp);
  auto parser = epi::Parser(tokenizer);

  try {
    parser.Parse();
    parser.Data();
  }
  catch (epi::cotyl::Exception& e) {
    if (!settings.catch_errors) throw;
    std::cerr << e.title << ':' << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    parser.PrintLoc(std::cerr);
    return -1;
  }

  auto emitter = epi::Emitter();
  try {
    emitter.MakeProgram(parser.declarations, parser.functions);
  }
  catch (epi::cotyl::Exception& e) {
    if (!settings.catch_errors) throw;
    std::cerr << e.title << ':' << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    return -1;
  }
  
  auto program = std::move(emitter.program);
//   epi::calyx::PrintProgram(program);

  for (auto& [sym, func] : program.functions) {
    // repeating multiple times will link more blocks
    auto func_hash = func.Hash();
    while (true) {
      std::cout << "Optimizing function " << sym.c_str() << " hash " << func_hash << "..." << std::endl;
      try {
        auto optimizer = epi::BasicOptimizer(std::move(func));
        func = optimizer.Optimize();

        auto new_hash = func.Hash();
        if (func_hash == new_hash) break;
        func_hash = new_hash;
      }
      catch (epi::cotyl::Exception& e) {
        if (!settings.catch_errors) throw;
        std::cerr << e.title << ':' << std::endl << std::endl;
        std::cerr << e.what() << std::endl;
        return -1;
      }
    }
  }

//   epi::calyx::PrintProgram(program);

  std::cout << std::endl << std::endl;
  std::cout << "-- globals" << std::endl;
  for (const auto& [symbol, global] : program.globals) {
    std::cout << symbol.c_str() << " " << stringify(global) << std::endl;
  }

  for (const auto& string : program.strings) {
    std::cout << '"' << string.c_str() << '"' << std::endl;
  }

  if (!settings.novisualize) {
    epi::calyx::VisualizeProgram(program, "output/program.pdf");
  }

  int returned = -1;
  try {
    epi::calyx::Interpreter interpreter{};
    std::cout << std::endl << std::endl;
    std::cout << "-- interpreted" << std::endl;
    returned = interpreter.Interpret(program);
    std::cout << "return " << returned << std::endl;

    // extract globals from interpreter
    std::cout << "  -- globals" << std::endl;
    for (const auto& [symbol, global] : interpreter.globals) {
      std::cout << "  " << symbol.c_str() << " " << stringify(global) << std::endl;
    }

    if (!settings.novisualize) {
      auto dependencies = epi::ProgramDependencies::GetDependencies(program);
      dependencies.VisualizeVars("output/vars.pdf");
    }
  }
  catch (epi::cotyl::Exception& e) {
    if (!settings.catch_errors) throw;
    std::cerr << e.title << ':' << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    return -1;
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
  if (!settings.novisualize) {
    rig.Visualize("output/rig.pdf");
  }

  return returned;
}


int main(int argc, char** argv) {
  auto settings = epi::info::parse_args(argc, argv);
  try {
    return parse_program(settings);
  }
  catch (std::runtime_error& e) {
    if (!settings.catch_errors) throw;
    std::cerr << "Uncaught exception:" << std::endl << std::endl;
    std::cerr << e.what() << std::endl;
    return -1;
  }
}
