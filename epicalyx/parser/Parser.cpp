#include "Parser.h"

#include "Exceptions.h"
#include "Stream.h"
#include "tokenizer/Token.h"
#include "types/Types.h"
#include "ast/Declaration.h"
#include "ast/Statement.h"

#include <iostream>


namespace epi {

using namespace ast;

void ConstParser::PrintLoc() const {
  in_stream.PrintLoc();
}

type::AnyType Parser::ResolveIdentifierType(const cotyl::CString& name) const {
  if (enum_values.Has(name)) {
    return type::ValueType<Parser::enum_type>(
            enum_values.Get(name),
            type::BaseType::LValueNess::None,
            type::BaseType::Qualifier::Const
    );
  }
  else if (variables.Has(name)) {
    return variables.Get(name);
  }
  else {
    throw cotyl::FormatExceptStr("Undeclared identifier: '%s'", name);
  }
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
    cotyl::vector<pNode<DeclarationNode>> decls{};
    auto function = ExternalDeclaration(decls);

    if (variables.Depth() != 1) [[unlikely]] {
      throw cotyl::FormatExceptStr("Scope not handled properly: depth %s", variables.Depth());
    }

    if (function) {
      RecordDeclaration(*function);
      if (!decls.empty()) {
        throw std::runtime_error("Bad parsing: unexpected declaration");
      }
      declarations.emplace_back(std::move(function));
    }
    else {
      for (auto& decl : decls) {
        RecordDeclaration(*decl);
        declarations.emplace_back(std::move(decl));
      }
    }
  }

  for (const auto& label : unresolved_labels) {
    if (!labels.contains(label)) {
      throw cotyl::FormatExceptStr("Unresolved label: %s", label);
    }
  }
}

void Parser::Data() {
  std::cout << "-- struct / union defs" << std::endl;
  for (const auto& def : structdefs.Base()) {
    std::cout << def.first.c_str() << " : " << stringify(def.second) << std::endl;
  }
  for (const auto& def : uniondefs.Base()) {
    std::cout << def.first.c_str() << " : " << stringify(def.second) << std::endl;
  }
  std::cout << std::endl << "-- typedefs" << std::endl;
  for (const auto& def : typedefs.Base()) {
    std::cout << def.first.c_str() << " : " << stringify(def.second) << std::endl;
  }
  std::cout << std::endl << "-- enum values" << std::endl;
  for (const auto& def : enum_values.Base()) {
    std::cout << def.first.c_str() << " : " << def.second << std::endl;
  }

  std::cout << std::endl << "-- declarations" << std::endl;
  for (const auto& decl : declarations) {
    std::cout << stringify(decl) << std::endl;
  }
}

}