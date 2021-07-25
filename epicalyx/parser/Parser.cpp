#include "Parser.h"

#include "nodes/Declaration.h"
#include "nodes/Statement.h"

#include <iostream>


namespace epi {

Parser::Parser(cotyl::pStream<Token>& in_stream) :
        in_stream(in_stream) {

}

void Parser::Parse() {
  while (!in_stream.EOS()) {
    auto function = ExternalDeclaration(declarations);
    if (function) {
      functions.push_back(std::move(function));
    }
  }
}


void Parser::Data() {
  std::cout << "-- struct / union defs" << std::endl;
  for (const auto& def : structdefs.Base()) {
    std::cout << def.first << " : " << def.second->to_string() << std::endl;
  }
  for (const auto& def : uniondefs.Base()) {
    std::cout << def.first << " : " << def.second->to_string() << std::endl;
  }
  std::cout << std::endl << "-- typedefs" << std::endl;
  for (const auto& def : typedefs.Base()) {
    std::cout << def.first << " : " << def.second->to_string() << std::endl;
  }
  std::cout << std::endl << "-- enum values" << std::endl;
  for (const auto& def : enum_values.Base()) {
    std::cout << def.first << " : " << def.second << std::endl;
  }

  std::cout << std::endl << "-- declarations" << std::endl;
  for (const auto& decl : declarations) {
    std::cout << decl->to_string() << std::endl;
  }
  std::cout << std::endl << "-- functions" << std::endl;
  for (const auto& func : functions) {
    std::cout << func->to_string() << std::endl << std::endl;
  }
}

}