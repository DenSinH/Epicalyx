#include "Parser.h"
#include "Is.h"
#include "Stream.h"
#include "tokenizer/Token.h"
#include "types/AnyType.h"
#include "ast/Declaration.h"
#include "ast/Statement.h"

#include <optional>
#include <iostream>


namespace epi {

using namespace ast;


bool Parser::IsDeclarationSpecifier(int after) {
  const Token* current;
  if (!in_stream.Peek(current, after)) {
    return false;
  }
  switch (current->type) {
    // storage class specifiers
    case TokenType::Typedef:
    case TokenType::Extern:
    case TokenType::Static:
    case TokenType::ThreadLocal:
    case TokenType::Auto:
    case TokenType::Register:
    // type specifiers
    case TokenType::Void:
    case TokenType::Char:
    case TokenType::Short:
    case TokenType::Int:
    case TokenType::Long:
    case TokenType::Float:
    case TokenType::Double:
    case TokenType::Signed:
    case TokenType::Unsigned:
    case TokenType::Bool:
    case TokenType::Complex:
    case TokenType::Atomic:
    case TokenType::Struct:
    case TokenType::Union:
    case TokenType::Enum:
    // function specifiers
    case TokenType::Inline:
    case TokenType::Noreturn:
    // type qualifiers
    case TokenType::Const:
    case TokenType::Restrict:
    case TokenType::Volatile:
    // alignment specifier
    case TokenType::Alignas: {
      return true;
    }
    case TokenType::Identifier: {
      // if typedef name exists: true
      return typedefs.Has(static_cast<const IdentifierToken*>(current)->name);
    }
    default:
      return false;
  }
}

static void MergeStructUnionDef(type::StructUnionType& dest, type::StructUnionType&& src) {
  if (dest.fields.empty()) {
    dest.fields = std::move(src.fields);
  }
  else {
    if (!dest.TypeEqualImpl(src)) {
      throw cotyl::FormatExcept<ParserError>("Redefinition of struct %s", dest.name.c_str());
    }
  }
}

static void MergeStructUnionDefs(type::StructUnionType& dest, type::StructUnionType& src) {
  if (dest.fields.empty()) {
    std::copy(src.fields.begin(), src.fields.end(), std::back_inserter(dest.fields));
  }
  else if (src.fields.empty()) {
    std::copy(dest.fields.begin(), dest.fields.end(), std::back_inserter(src.fields));
  }
  else {
    if (!dest.TypeEqualImpl(src)) {
      throw cotyl::FormatExcept<ParserError>("Redefinition of struct %s", dest.name.c_str());
    }
  }
}

static void MergeTypes(type::AnyType& existing, type::AnyType& found) {
  if (!existing.TypeEquals(found) || existing->qualifiers != found->qualifiers) {
    throw cotyl::FormatExceptStr<ParserError>("Type mismatch: %s vs %s", existing, found);
  }
  
  // override incomplete struct definition
  if (existing.holds_alternative<type::StructType>() && existing.get<type::StructType>().fields.empty()) {
    MergeStructUnionDefs(existing.get<type::StructType>(), found.get<type::StructType>());
  }
  if (existing.holds_alternative<type::UnionType>() && existing.get<type::UnionType>().fields.empty()) {
    MergeStructUnionDefs(existing.get<type::UnionType>(), found.get<type::UnionType>());
  }
}

void Parser::RecordDeclaration(const cotyl::CString& name, type::AnyType& type) {
  if (name.empty()) {
    return;
  }
  
  if (typedefs.HasTop(name) || structdefs.HasTop(name) || uniondefs.HasTop(name) || enums.HasTop(name)) {
    throw cotyl::FormatExcept<ParserError>("Redefinition of symbol %s", name.c_str());
  }
  else if (variables.HasTop(name)) {
    // gets the first scoped value (which will be the top one)
    auto& existing = variables.Get(name);
    MergeTypes(existing, type);
  }
  else {
    variables.Set(name, type);
  }
}

void Parser::DStaticAssert() {
  in_stream.EatSequence(TokenType::StaticAssert, TokenType::LParen);
  auto expr = Parser::EConstexpr();
  in_stream.Eat(TokenType::Comma);
  auto strt = in_stream.Expect(TokenType::StringConstant);
  auto str = std::move(strt.get<StringConstantToken>().value);
  in_stream.EatSequence(TokenType::RParen, TokenType::SemiColon);

  if (!expr) {
    throw cotyl::FormatExcept<ParserError>("Static assertion failed: %s", str.c_str());
  }
}

type::AnyType Parser::DEnum() {
  // return type with type::LValue::Assignable by default,
  // since this will be used to initialize variables with 
  // enum type
  in_stream.Eat(TokenType::Enum);
  cotyl::CString name;
  if (in_stream.IsAfter(0, TokenType::Identifier)) {
    // enum name
    name = std::move(in_stream.Get().get<IdentifierToken>().name);
    if (!in_stream.EatIf(TokenType::LBrace)) {
      // check if enum was defined before
      if (!enums.Has(name)) {
        throw cotyl::FormatExcept<ParserError>("Undefined enum %s", name.c_str());
      }
      return type::ValueType<enum_type>(type::LValue::Assignable);
    }
  }
  else {
    in_stream.Eat(TokenType::LBrace);
  }
  // enum <name> { ... }

  enum_type counter = 0;
  do {
    auto idt = in_stream.Expect(TokenType::Identifier);
    auto constant = std::move(idt.get<IdentifierToken>().name);
    if (in_stream.EatIf(TokenType::Assign)) {
      // constant = value
      // update counter
      counter = EConstexpr();
    }
    enum_values.Set(constant, counter);
    variables.Set(constant, type::ValueType<enum_type>(counter, type::LValue::None, type::Qualifier::Const));
    counter++;
    if (!in_stream.EatIf(TokenType::Comma)) {
      // no comma: expect end of enum declaration
      in_stream.Eat(TokenType::RBrace);
      break;
    }
  } while(!in_stream.EatIf(TokenType::RBrace));

  // only add if not anonymous
  if (!name.empty()) enums.Add(name);
  return type::ValueType<enum_type>(type::LValue::Assignable);
}

type::AnyType Parser::DStruct() {
  cotyl::CString name;
  bool is_struct = true;
  if (!in_stream.EatIf(TokenType::Struct)) {
    in_stream.Eat(TokenType::Union);
    is_struct = false;
  }

  if (in_stream.IsAfter(0, TokenType::Identifier)) {
    name = std::move(in_stream.Get().get<IdentifierToken>().name);
    if (!in_stream.EatIf(TokenType::LBrace)) {
      if (is_struct) {
        if (!structdefs.Has(name)) {
          auto fwddef = type::StructType{
            std::move(name), {}, type::LValue::Assignable
          };
          return structdefs.Set(fwddef.name, std::move(fwddef));
        }
        return structdefs.Get(name);
      }
      else {
        if (!structdefs.Has(name)) {
          auto fwddef = type::UnionType{
            std::move(name), {}, type::LValue::Assignable
          };
          return uniondefs.Set(fwddef.name, std::move(fwddef));
        }
        return uniondefs.Get(name);
      }
    }
  }
  else {
    in_stream.Eat(TokenType::LBrace);
  }

  cotyl::vector<type::StructField> fields{};
  while(!in_stream.EatIf(TokenType::RBrace)) {
    if (in_stream.IsAfter(0, TokenType::StaticAssert)) {
      DStaticAssert();
    }
    else {
      auto ctype = DSpecifier();

      do {
        auto decl = DDeclarator(ctype.first, ctype.second);
        size_t size = 0;
        if (decl.storage != StorageClass::None) {
          throw ParserError("Invalid storage class specifier in struct definition");
        }
        if (in_stream.EatIf(TokenType::Colon)) {
          size = EConstexpr();
        }
        fields.emplace_back(std::move(decl.name), size, std::make_unique<type::AnyType>(std::move(decl.type)));
      } while (in_stream.EatIf(TokenType::Comma));
      in_stream.Eat(TokenType::SemiColon);
    }
  }

  if (is_struct) {
    auto result_type = type::StructType{
      std::move(name),
      std::move(fields),
      type::LValue::Assignable
    };
    if (!result_type.name.empty()) {
      if (structdefs.HasTop(result_type.name)) {
        auto& existing = structdefs.Get(result_type.name);
        MergeStructUnionDef(existing, std::move(result_type));
        return existing;
      }
      else {
        structdefs.Set(result_type.name, result_type);
        return result_type;
      }
    }
    return std::move(result_type);
  }
  else {
    auto result_type = type::UnionType{
      std::move(name),
      std::move(fields),
      type::LValue::Assignable
    };
    if (!result_type.name.empty()) {
      if (uniondefs.HasTop(result_type.name)) {
        auto& existing = uniondefs.Get(result_type.name);
        MergeStructUnionDef(existing, std::move(result_type));
        return existing;
      }
      else {
        uniondefs.Set(result_type.name, result_type);
        return result_type;
      }
    }
    return std::move(result_type);
  }
}

std::pair<type::AnyType, StorageClass> Parser::DSpecifier() {
  std::optional<StorageClass> storage{};
  enum class Type {
    Void, Char, Short, Long, LongLong, Int, ShortInt, LongInt, LongLongInt, Float, Double
  };

  std::optional<type::AnyType> ctype;

  std::optional<Type> type{};
  std::optional<int> sign{};
  u32 qualifiers = 0;
  bool is_function = false;

  auto set_storage = [&storage](StorageClass s) {
    if (storage.has_value()) {
      throw ParserError("Double storage class specifier in declaration");
    }
    storage = s;
  };

  auto assert_no_type = [&type]() {
    if (type.has_value()) {
      throw ParserError("Double type specifier in declaration");
    }
  };

  auto assert_no_ctype = [&ctype]() {
    if (ctype.has_value()) {
      throw ParserError("Double type in declaration");
    }
  };

  auto assert_no_sign = [&sign]() {
    if (sign.has_value()) {
      throw ParserError("Double type specifier in declaration");
    }
  };

  const Token* current;

  bool was_specifier = true;
  do {
    current = in_stream.ForcePeek();

    switch (current->type) {
      case TokenType::Typedef:     in_stream.Skip(); set_storage(StorageClass::Typedef); break;
      case TokenType::Extern:      in_stream.Skip(); set_storage(StorageClass::Extern); break;
      case TokenType::Static:      in_stream.Skip(); set_storage(StorageClass::Static); break;
      case TokenType::ThreadLocal: in_stream.Skip(); set_storage(StorageClass::ThreadLocal); break;
      case TokenType::Auto:        in_stream.Skip(); set_storage(StorageClass::Auto); break;
      case TokenType::Register:    in_stream.Skip(); set_storage(StorageClass::Register); break;

      case TokenType::Void: {
        assert_no_type();
        assert_no_sign();
        in_stream.Skip();
        type = Type::Void;
        break;
      }
      case TokenType::Char: {
        assert_no_type();
        in_stream.Skip();
        type = Type::Char;
        break;
      }
      case TokenType::Short: {
        in_stream.Skip();
        if (type.has_value()) {
          if (type.value() == Type::Int) {
            type = Type::ShortInt;
          }
          else {
            assert_no_type();
          }
        }
        else {
          type = Type::Short;
        }
        break;
      }
      case TokenType::Long: {
        in_stream.Skip();
        if (type.has_value()) {
          switch (type.value()) {
            case Type::Int: type = Type::LongInt; break;
            case Type::Long: type = Type::LongLong; break;
            case Type::LongInt: type = Type::LongLongInt; break;
            case Type::Double: throw cotyl::UnimplementedException("long double type");
            default:
              assert_no_type();
          }
        }
        else {
          type = Type::Long;
        }
        break;
      }
      case TokenType::Int: {
        in_stream.Skip();
        if (type.has_value()) {
          switch (type.value()) {
            case Type::Short: type = Type::ShortInt; break;
            case Type::Long: type = Type::LongInt; break;
            case Type::LongLong: type = Type::LongLongInt; break;
            default:
              assert_no_type();
          }
        }
        else {
          type = Type::Int;
        }
        break;
      }
      case TokenType::Float: {
        assert_no_type();
        in_stream.Skip();
        type = Type::Float;
        break;
      }
      case TokenType::Double: {
        in_stream.Skip();
        if (type.has_value() && type.value() == Type::Long) {
          throw cotyl::UnimplementedException("long double");
        }
        type = Type::Double;
        break;
      }
      case TokenType::Bool: {
        in_stream.Skip();
        throw cotyl::UnimplementedException("_Bool");
      }
      case TokenType::Complex: {
        in_stream.Skip();
        throw cotyl::UnimplementedException("_Complex");
      }

      case TokenType::Struct:
      case TokenType::Union: {
        assert_no_type();
        assert_no_sign();
        assert_no_ctype();
        ctype.emplace(DStruct());
        break;
      }
      case TokenType::Enum: {
        assert_no_type();
        assert_no_sign();
        assert_no_ctype();
        ctype.emplace(DEnum());
        break;
      }

      case TokenType::Signed: {
        in_stream.Skip();
        if (!sign.has_value() || sign.value() == -1) {
          if (type.has_value() && !cotyl::Is(type.value()).AnyOf<
                  Type::Char, Type::Short, Type::Int, Type::Long, Type::LongLong,
                  Type::ShortInt, Type::LongInt, Type::LongLongInt>()) {
            throw ParserError("Invalid sign specifier for type");
          }
          // warn: double sign specifier?
          sign = -1;
        }
        else {
          throw ParserError("Cannot use signed and unsigned in a single declaration");
        }
        break;
      }
      case TokenType::Unsigned: {
        in_stream.Skip();
        if (!sign.has_value() || sign.value() == 1) {
          if (type.has_value() && !cotyl::Is(type.value()).AnyOf<
                  Type::Char, Type::Short, Type::Int, Type::Long, Type::LongLong,
                  Type::ShortInt, Type::LongInt, Type::LongLongInt>()) {
            throw ParserError("Invalid sign specifier for type");
          }
          // warn: double sign specifier?
          sign = 1;
        }
        else {
          throw ParserError("Cannot use signed and unsigned in a single declaration");
        }
        break;
      }

      case TokenType::Const:    in_stream.Skip(); qualifiers |= type::Qualifier::Const; break;
      case TokenType::Restrict: in_stream.Skip(); qualifiers |= type::Qualifier::Restrict; break;
      case TokenType::Volatile: in_stream.Skip(); qualifiers |= type::Qualifier::Volatile; break;
      case TokenType::Atomic: {
        in_stream.Skip();
        if (in_stream.IsAfter(1, TokenType::LParen)) {
          throw cotyl::UnimplementedException("_Atomic");
        }
        qualifiers |= type::Qualifier::Atomic;
        break;
      }

      case TokenType::Inline:
      case TokenType::Noreturn: {
        in_stream.Skip();
        // todo
        is_function = true;
        break;
      }

      case TokenType::Alignas: {
        in_stream.Skip();
        throw cotyl::UnimplementedException("_Alignas");
      }

      case TokenType::Identifier: {
        const auto& ident_name = static_cast<const IdentifierToken*>(current)->name;
        if (typedefs.Has(ident_name)) {
          if (ctype) {
            // repeat typedef declaration
            was_specifier = false;
          }
          else {
            ctype.emplace(typedefs.Get(ident_name));
            in_stream.Skip();
          }
        }
        else {
          // otherwise: name is detected in declarator, was not a specifier
          was_specifier = false;
        }
        break;
      }

      default: {
        was_specifier = false;
        break;
      }
    }
  } while (was_specifier);


  auto make = [=]<typename T>() -> type::AnyType {
    if (sign.value_or(-1) == -1) return type::ValueType<T>(type::LValue::Assignable, qualifiers);
    return type::ValueType<std::make_unsigned_t<T>>(type::LValue::Assignable, qualifiers);
  };

  if (!type.has_value()) {
    if (!ctype) {
      // ctype might have been set by a typedef name
      ctype.emplace(make.operator()<i32>());
    }
  }
  else {
    if (ctype) {
      // ctype might have been set by a typedef name
      throw ParserError("Invalid type specifier");
    }

    switch (type.value()) {
      case Type::Int: case Type::LongInt: case Type::Long:
        ctype.emplace(make.operator()<i32>()); break;
      case Type::Short: case Type::ShortInt:
        ctype.emplace(make.operator()<i16>()); break;
      case Type::LongLong: case Type::LongLongInt:
        ctype.emplace(make.operator()<i64>()); break;
      case Type::Char:
        ctype.emplace(make.operator()<i8>()); break;
      case Type::Void:
        ctype.emplace(type::VoidType(qualifiers)); break;
      case Type::Float:
        ctype.emplace(type::ValueType<float>(type::LValue::Assignable, qualifiers)); break;
      case Type::Double:
        ctype.emplace(type::ValueType<double>(type::LValue::Assignable, qualifiers)); break;
    }
  }
  return std::make_pair(ctype.value(), storage ? storage.value() : StorageClass::None);
}

template<> Parser::any_pointer_t::~Variant() = default;

cotyl::CString Parser::DDirectDeclaratorImpl(std::stack<any_pointer_t>& dest) {
  cotyl::CString name;
  std::stack<any_pointer_t> layer{};

  const Token* current;
  while (true) {
    in_stream.Peek(current);
    // nested (abstract) declarators
    switch (current->type) {
      case TokenType::Asterisk: {
        // pointer with qualifiers
        in_stream.Skip();
        u8 ptr_qualifiers = 0;
        while (in_stream.IsAfter(0, TokenType::Const, TokenType::Restrict, TokenType::Volatile, TokenType::Atomic)) {
          switch (in_stream.Get()->type) {
            case TokenType::Const: ptr_qualifiers |= type::Qualifier::Const; break;
            case TokenType::Restrict: ptr_qualifiers |= type::Qualifier::Restrict; break;
            case TokenType::Volatile: ptr_qualifiers |= type::Qualifier::Volatile; break;
            case TokenType::Atomic: ptr_qualifiers |= type::Qualifier::Atomic; break;
            default:
              // [[unreachable]]
              break;
          }
        }
        layer.push(type::PointerType{nullptr, type::LValue::Assignable, ptr_qualifiers});
        break;
      }
      case TokenType::LParen: {
        // either nested declarator or function type
        // function if the next token is a declaration specifier or a closing parenthesis
        // not a function if the next token is another parenthesis or an identifier that is not a typedef name
        in_stream.Skip();
        switch (in_stream.ForcePeek()->type) {
          case TokenType::Void: {
            if (in_stream.IsAfter(1, TokenType::RParen)) {
              // function(void)
              in_stream.Skip();
              [[fallthrough]];
            }
            else {
              goto function_call;
            }
          }
          case TokenType::RParen: {
            // function()
            layer.push(type::FunctionType{nullptr, false, type::LValue::Assignable});
            in_stream.Skip();
            break;
          }
          case TokenType::LParen: {
            // ((direct-declarator))
            in_stream.Skip();
            auto _name = DDirectDeclaratorImpl(dest);
            if (!name.empty() && !_name.empty() ) {
              throw cotyl::FormatExceptStr<ParserError>("Double name in declaration (%s and %s)", name, _name);
            }
            else if (name.empty()) {
              name = std::move(_name);
            }
            in_stream.EatSequence(TokenType::RParen, TokenType::RParen);
            break;
          }
          case TokenType::Asterisk: {
            // (*direct-declarator)
            auto _name = DDirectDeclaratorImpl(dest);
            if (!name.empty() && !_name.empty() ) {
              throw cotyl::FormatExceptStr<ParserError>("Double name in declaration (%s and %s)", name, _name);
            }
            else if (name.empty()) {
              name = std::move(_name);
            }
            in_stream.EatSequence(TokenType::RParen);
            break;
          }
          case TokenType::Identifier: {
            // (typedef name) or (name)
            auto ident_name = std::move(in_stream.Get().get<IdentifierToken>().name);
            if (!typedefs.Has(ident_name)) {
              // if not typdef name: direct declarator name
              if (!name.empty()) {
                throw ParserError("Double name in declaration");
              }
              name = std::move(ident_name);
              break;
            }
            // else: (function argument type)
          } [[fallthrough]];
          default: {
            function_call:
            // has to be a function declaration with at least one parameter
            auto typ = type::FunctionType{nullptr, false, type::LValue::Assignable};

            do {
              auto arg_specifier = DSpecifier();
              if (!cotyl::Is(arg_specifier.second).AnyOf<StorageClass::None, StorageClass::Register>()) {
                throw ParserError("Bad storage specifier on function argument");
              }

              auto arg = DDeclarator(std::move(arg_specifier.first), StorageClass::Auto);
              typ.AddArg(std::move(arg.name), std::make_shared<type::AnyType>(std::move(arg.type)));
              if (in_stream.EatIf(TokenType::Comma)) {
                if (in_stream.EatIf(TokenType::Ellipsis)) {
                  typ.variadic = true;
                  in_stream.Eat(TokenType::RParen);
                  break;
                }
              }
              else {
                in_stream.Eat(TokenType::RParen);
                break;
              }
            } while (true);
            layer.push(std::move(typ));
          }
        }
        break;
      }
      case TokenType::Identifier: {
        if (!name.empty()) {
          throw ParserError("Double name in declaration");
        }
        name = std::move(in_stream.Get().get<IdentifierToken>().name);
        break;
      }
      case TokenType::LBracket: {
        // array type
        in_stream.Skip();
        size_t size = 0;
        while (in_stream.Peek(current) && current->type != TokenType::RBracket) {
          if (in_stream.IsAfter(0, TokenType::Const, TokenType::Restrict, TokenType::Volatile, TokenType::Atomic, TokenType::Static)) {
            in_stream.Skip();  // todo: weird array[static type-specifier] declaration
          }
          else {
            size = EConstexpr();
            break;
          }
        }
        in_stream.Eat(TokenType::RBracket);
        layer.push(type::ArrayType{nullptr, size});
        break;
      }
      default: {
        while (!layer.empty()) {
          // declarator might be empty
          // for example: int (a) = 0;
          dest.push(std::move(layer.top()));
          layer.pop();
        }
        return name;
      }
    }
  }
}

static type::AnyType UnwindDirectDeclarators(type::AnyType&& ctype, std::stack<Parser::any_pointer_t>& direct) {
  if (direct.empty()) {
    return std::move(ctype);
  }
  auto top = std::move(direct.top());
  direct.pop();
  top->contained = std::make_unique<type::AnyType>(std::move(ctype));
  
  return top.visit<type::AnyType>(
    [&](auto&& ptr) {
      return UnwindDirectDeclarators(std::move(ptr), direct);
    }
  );
}

DeclarationNode Parser::DDeclarator(type::AnyType ctype, StorageClass storage) {
  std::stack<any_pointer_t> direct{};

  cotyl::CString name = DDirectDeclaratorImpl(direct);
  auto apparent_type = UnwindDirectDeclarators(std::move(ctype), direct);
  return DeclarationNode(std::move(apparent_type), std::move(name), storage);
}

void Parser::DInitDeclaratorList(cotyl::vector<DeclarationNode>& dest) {
  auto [ctype, storage] = DSpecifier();
  do {
    StoreDeclaration(DDeclarator(ctype, storage), dest);
  } while (in_stream.EatIf(TokenType::Comma));
}

void Parser::StoreDeclaration(DeclarationNode&& decl, cotyl::vector<ast::DeclarationNode>& dest) {
  if (decl.storage == StorageClass::Typedef) {
    // store typedef names
    if (decl.name.empty()) {
      // Log::Warn("Useless typedef storage class declaration");
      return;
    }
    if (typedefs.HasTop(decl.name)) {
      auto& existing = typedefs.Get(decl.name);
      MergeTypes(existing, decl.type);
    }
    else {
      typedefs.Set(decl.name, decl.type);
    }
  }
  else {
    RecordDeclaration(decl.name, decl.type);
    if (in_stream.EatIf(TokenType::Assign)) {
      // type var = <expression> or {initializer list}
      if (decl.name.empty()) {
        throw ParserError("Cannot assign to nameless variable");
      }
      decl.value = EInitializer();
      dest.emplace_back(std::move(decl));
    }
    else {
      // type var, var2, var3
      if (!decl.name.empty()) {
        dest.emplace_back(std::move(decl));
      }
      else {
        // warn: statement has no effect
      }
    }
  }
}

void Parser::ExternalDeclaration() {
  if (in_stream.IsAfter(0, TokenType::StaticAssert)) {
    DStaticAssert();
    return;
  }
  auto [ctype, storage] = DSpecifier();
  auto decl = DDeclarator(ctype, storage);

  // signature { body }
  if (in_stream.EatIf(TokenType::LBrace)) {
    if (!decl.type.holds_alternative<type::FunctionType>()) {
      throw ParserError("Unexpected compound statement after external declaration");
    }
    type::FunctionType signature = std::move(decl.type.get<type::FunctionType>());
    signature.lvalue = type::LValue::LValue;

    auto symbol = std::move(decl.name);
    if (symbol.empty()) {
      throw ParserError("Missing name in function declaration");
    }
    
    // function has to be available within itself for recursion
    // we don't use RecordDeclaration because function types are special
    // and because we do not need to merge type type
    if (variables.HasTop(symbol)) {
      // gets the first scoped value (which will be the top one)
      auto& existing = variables.Get(symbol);
      if (!existing.TypeEquals(signature) || existing->qualifiers != signature.qualifiers) {
        throw cotyl::FormatExceptStr<ParserError>("Type mismatch: %s vs %s", existing, signature);
      }
    }
    else {
      variables.Set(symbol, signature);
    }

    // add new local layer for arguments
    variables.NewLayer();
    for (const auto& arg : signature.arg_types) {
      if (arg.name.empty()) {
        throw ParserError("Nameless argument in function definition");
      }
      variables.Set(arg.name, *arg.type);
    }
    
    cotyl::Assert(!function_return && !function_symbol);
    function_return = signature.contained.get();
    function_symbol = symbol.c_str();
    auto body = SCompound();
    function_return = nullptr;
    function_symbol = nullptr;
    variables.PopLayer();
    in_stream.Eat(TokenType::RBrace);
    
    auto func = FunctionDefinitionNode(
      std::move(signature), std::move(symbol), std::move(body)
    );
    functions.emplace_back(std::move(func));
  }
  else {
    // normal declaration
    StoreDeclaration(std::move(decl), declarations);
    while (!in_stream.EatIf(TokenType::SemiColon)) {
      in_stream.Eat(TokenType::Comma);
      auto decl = DDeclarator(ctype, storage);
      StoreDeclaration(std::move(decl), declarations);
    }
  }
}

}