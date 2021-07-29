#include "Parser.h"

#include "nodes/Declaration.h"
#include "nodes/Statement.h"

#include <iostream>


namespace epi {

Parser::Parser(cotyl::Stream<pToken>& in_stream) :
        in_stream(in_stream) {

}

void Parser::PrintLoc() const {
  in_stream.PrintLoc();
}

void Parser::PushScope() {
  variables.NewLayer();
  typedefs.NewLayer();
  structdefs.NewLayer();
  uniondefs.NewLayer();
  enums.NewLayer();
  enum_values.NewLayer();
}

void Parser::PopScope() {
  variables.PopLayer();
  typedefs.PopLayer();
  structdefs.PopLayer();
  uniondefs.PopLayer();
  enums.PopLayer();
  enum_values.PopLayer();
}

void Parser::Parse() {
  while (!in_stream.EOS()) {
    std::vector<pNode<Declaration>> decls;
    auto function = ExternalDeclaration(decls);

    if (variables.Depth() != 1) [[unlikely]] {
      throw std::runtime_error("Scope not handled properly");
    }

    if (function) {
      function->VerifyAndRecord(*this);
      if (!decls.empty()) {
        throw std::runtime_error("Bad parsing: unexpected declaration");
      }
      functions.push_back(std::move(function));
    }
    else {
      for (auto& decl : decls) {
        decl->VerifyAndRecord(*this);
        declarations.push_back(std::move(decl));
      }
    }
  }
}

void Parser::Data() {
  std::cout << "-- struct / union defs" << std::endl;
  for (const auto& def : structdefs.Base()) {
    std::cout << def.first << " : " << def.second->ToString() << std::endl;
  }
  for (const auto& def : uniondefs.Base()) {
    std::cout << def.first << " : " << def.second->ToString() << std::endl;
  }
  std::cout << std::endl << "-- typedefs" << std::endl;
  for (const auto& def : typedefs.Base()) {
    std::cout << def.first << " : " << def.second->ToString() << std::endl;
  }
  std::cout << std::endl << "-- enum values" << std::endl;
  for (const auto& def : enum_values.Base()) {
    std::cout << def.first << " : " << def.second << std::endl;
  }

  std::cout << std::endl << "-- declarations" << std::endl;
  for (const auto& decl : declarations) {
    std::cout << decl->ToString() << std::endl;
  }
  std::cout << std::endl << "-- functions" << std::endl;
  for (const auto& func : functions) {
    std::cout << func->ToString() << std::endl << std::endl;
  }
}

}