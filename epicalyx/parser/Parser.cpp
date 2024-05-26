#include "Parser.h"

#include <iostream>                    // for basic_ostream, char_traits
#include <string>                      // for operator<<

#include "Vector.h"                    // for vec_iterator
#include "CustomAssert.h"              // for Assert
#include "Format.h"                    // for FormatExceptStr
#include "Stream.h"                    // for Stream
#include "Stringify.h"                 // for stringify
#include "ast/Declaration.h"           // for DeclarationNode
#include "tokenizer/Token.h"           // for AnyToken
#include "types/Types.h"               // for StructType, UnionType


namespace epi {

using namespace ast;

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
      throw cotyl::FormatExceptStr<ParserError>("Unresolved label: %s", label);
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