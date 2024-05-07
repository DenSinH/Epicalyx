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
#include "Locatable.h"


#include "Log.h"

using ::epi::stringify;

struct SafeRun {
  SafeRun(bool catch_errors, const epi::cotyl::Locatable* loc = nullptr) : 
      catch_errors{catch_errors}, loc{loc} { }

  template<typename F>
  void operator<<(F&& callable) {
    if (!catch_errors) {
      callable();
    }
    else {
      try {
        callable();
      }
      catch (epi::cotyl::Exception& e) {
        std::cerr << e.title << ':' << std::endl << std::endl;
        std::cerr << e.what() << std::endl;
        if (loc) loc->PrintLoc(std::cerr);
        std::exit(-1);
      }
      catch (std::runtime_error& e) {
        std::cerr << "Compiler Error:" << std::endl << std::endl;
        std::cerr << e.what() << std::endl;
        std::exit(-1);
      }
      catch (...) {
        std::cerr << "Unknown Compiler Error" << std::endl;
        std::exit(-1);
      }
    }
  }

private:
  bool catch_errors = false;
  const epi::cotyl::Locatable* loc = nullptr;
};


int main(int argc, char** argv) {
  auto settings = epi::info::parse_args(argc, argv);
  auto rig_func_sym = epi::cotyl::CString(settings.rigfunc);
  bool ce = settings.catch_errors;


  std::unique_ptr<epi::Preprocessor> pp; 
  SafeRun(ce) << [&]{ 
    pp = std::make_unique<epi::Preprocessor>(settings.filename);
    if (!settings.stl.empty()) {
      pp->SetSTLPath(settings.stl);
    } 
  };

  while (!pp->EOS()) std::cout << pp->Get() << std::flush;
  return 0;

  auto tokenizer = std::make_unique<epi::Tokenizer>(*pp);
  auto parser = std::make_unique<epi::Parser>(*tokenizer);

  SafeRun(ce, parser.get()) << [&]{
    parser->Parse();
    parser->Data();
  };

  auto emitter = epi::Emitter();
  SafeRun(ce) << [&]{
    emitter.MakeProgram(parser->declarations, parser->functions);
  };

  auto program = std::move(emitter.program);
//   epi::calyx::PrintProgram(program);

  for (auto& [sym, func] : program.functions) {
    // repeating multiple times will link more blocks
    auto func_hash = func.Hash();
    while (true) {
      std::cout << "Optimizing function " << sym.c_str() << " hash " << func_hash << "..." << std::endl;
      SafeRun(ce) << [&]{
        auto optimizer = epi::BasicOptimizer(std::move(func));
        func = optimizer.Optimize();
      };

      auto new_hash = func.Hash();
      if (func_hash == new_hash) break;
      func_hash = new_hash;
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
  SafeRun(ce) << [&]{
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
  };

  if (!program.functions.contains(rig_func_sym)) {
    std::cout << "No function " << rig_func_sym.view()
              << " exists, defaulting to 'main'..." << std::endl;
    rig_func_sym = epi::cotyl::CString("main");
  }
  if (program.functions.contains(rig_func_sym)) {
    SafeRun(ce) << [&]{
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
          case epi::ExampleRegSpace::RegType::Stack: std::cout << "Stack"; break;
        }
        std::cout << std::endl;
      }

      rig.Reduce(*regspace);
      if (!settings.novisualize) {
        rig.Visualize("output/rig.pdf");
      }
    };
  }

  return returned;
}
