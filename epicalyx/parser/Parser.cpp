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
    ExternalDeclaration();

    cotyl::Assert(variables.Depth() == 1, "Scope not handled properly");
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