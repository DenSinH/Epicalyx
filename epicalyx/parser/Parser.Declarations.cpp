#include "Parser.h"
#include "Is.h"
#include "Stream.h"
#include "tokenizer/Token.h"
#include "types/AnyType.h"
#include "ast/Declaration.h"
#include "ast/Statement.h"

#include <optional>


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

void Parser::RecordDeclaration(const cotyl::CString& name, const type::AnyType& type) {
  if (name.empty()) {
    return;
  }
  // todo: check enum/struct/typdef
  if (typedefs.HasTop(name) || structdefs.HasTop(name) || uniondefs.HasTop(name) || enums.HasTop(name)) {
    throw cotyl::FormatExcept("Redefinition of symbol %s", name.c_str());
  }
  else if (variables.HasTop(name)) {
    // gets the first scoped value (which will be the top one)
    const auto& existing = variables.Get(name);
    if (!existing.TypeEquals(type) || existing->qualifiers != type->qualifiers) {
      throw cotyl::FormatExcept("Redefinition of symbol %s", name.c_str());
    }
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
    throw cotyl::FormatExcept("Static assertion failed: %s", str.c_str());
  }
}

type::AnyType Parser::DEnum() {
  in_stream.Eat(TokenType::Enum);
  cotyl::CString name;
  if (in_stream.IsAfter(0, TokenType::Identifier)) {
    // enum name
    name = std::move(in_stream.Get().get<IdentifierToken>().name);
    if (!in_stream.EatIf(TokenType::LBrace)) {
      // check if enum was defined before
      if (!enums.Has(name)) {
        throw cotyl::FormatExcept("Undefined enum %s", name.c_str());
      }
      return type::ValueType<enum_type>(type::LValue::None);
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
  return type::ValueType<enum_type>(type::LValue::None);
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
        return structdefs.Get(name);
      }
      else {
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
          throw std::runtime_error("Invalid storage class specifier in struct definition");
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
    if (!result_type.name.empty()) structdefs.Set(result_type.name, result_type);
    return result_type;
  }
  else {
    auto result_type = type::UnionType{
      std::move(name),
      std::move(fields),
      type::LValue::Assignable
    };
    if (!result_type.name.empty()) uniondefs.Set(result_type.name, result_type);
    return result_type;
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
      throw std::runtime_error("Double storage class specifier in declaration");
    }
    storage = s;
  };

  auto assert_no_type = [&type]() {
    if (type.has_value()) {
      throw std::runtime_error("Double type specifier in declaration");
    }
  };

  auto assert_no_ctype = [&ctype]() {
    if (ctype.has_value()) {
      throw std::runtime_error("Double type in declaration");
    }
  };

  auto assert_no_sign = [&sign]() {
    if (sign.has_value()) {
      throw std::runtime_error("Double type specifier in declaration");
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
            case Type::Double: throw std::runtime_error("Unimplemented type: long double");
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
            throw std::runtime_error("Invalid sign specifier for type");
          }
          // warn: double sign specifier?
          sign = -1;
        }
        else {
          throw std::runtime_error("Cannot use signed and unsigned in a single declaration");
        }
        break;
      }
      case TokenType::Unsigned: {
        in_stream.Skip();
        if (!sign.has_value() || sign.value() == 1) {
          if (type.has_value() && !cotyl::Is(type.value()).AnyOf<
                  Type::Char, Type::Short, Type::Int, Type::Long, Type::LongLong,
                  Type::ShortInt, Type::LongInt, Type::LongLongInt>()) {
            throw std::runtime_error("Invalid sign specifier for type");
          }
          // warn: double sign specifier?
          sign = 1;
        }
        else {
          throw std::runtime_error("Cannot use signed and unsigned in a single declaration");
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
            throw std::runtime_error("Bad declaration");
          }
          ctype.emplace(typedefs.Get(ident_name));
          in_stream.Skip();
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

  int _sign = -1;
  if (sign.has_value()) {
    if (ctype) {
      throw std::runtime_error("Cannot specify sign for pointer type");
    }
    _sign = sign.value();
  }

  auto make = [=]<typename T>() -> type::AnyType {
    if (_sign == -1) return type::ValueType<T>(type::LValue::Assignable, qualifiers);
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
      throw std::runtime_error("Invalid type specifier");
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
  // std::stack<any_pointer_t> layer{};

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
        dest.push(type::PointerType{nullptr, type::LValue::Assignable, ptr_qualifiers});
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
            dest.push(type::FunctionType{nullptr, false, type::LValue::Assignable});
            in_stream.Skip();
            break;
          }
          case TokenType::LParen: {
            // ((direct-declarator))
            in_stream.Skip();
            auto _name = DDirectDeclaratorImpl(dest);
            if (!name.empty() && !_name.empty() ) {
              throw cotyl::FormatExceptStr("Double name in declaration (%s and %s)", name, _name);
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
              throw cotyl::FormatExceptStr("Double name in declaration (%s and %s)", name, _name);
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
                throw std::runtime_error("Double name in declaration");
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
              if (arg_specifier.second != StorageClass::None) {
                throw std::runtime_error("Bad storage specifier on function argument");
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
            dest.push(std::move(typ));
          }
        }
        break;
      }
      case TokenType::Identifier: {
        if (!name.empty()) {
          throw std::runtime_error("Double name in declaration");
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
        dest.push(type::PointerType{type::PointerType::ArrayType(nullptr, size)});
        break;
      }
      default: {
        // while (!layer.empty()) {
        //   // declarator might be empty
        //   // for example: int (a) = 0;
        //   dest.push(std::move(layer.top()));
        //   layer.pop();
        // }
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
      throw std::runtime_error("Typedef declaration must have a name");
    }
    if (typedefs.HasTop(decl.name)) {
      throw std::runtime_error("Redefinition of type alias");
    }
    typedefs.Set(decl.name, decl.type);
  }
  else {
    RecordDeclaration(decl.name, decl.type);
    if (in_stream.EatIf(TokenType::Assign)) {
      // type var = <expression> or {initializer list}
      if (decl.name.empty()) {
        throw std::runtime_error("Cannot assign to nameless variable");
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
      throw std::runtime_error("Unexpected compound statement after external declaration");
    }
    type::FunctionType signature = std::move(decl.type.get<type::FunctionType>());
    signature.lvalue = type::LValue::None;

    auto symbol = std::move(decl.name);
    if (symbol.empty()) {
      throw std::runtime_error("Missing name in function declaration");
    }
    
    // function has to be available within itself for recursion
    RecordDeclaration(symbol, signature);

    // add new local layer for arguments
    variables.NewLayer();
    for (const auto& arg : signature.arg_types) {
      if (arg.name.empty()) {
        throw std::runtime_error("Nameless argument in function definition");
      }
      variables.Set(arg.name, *arg.type);
    }
    
    cotyl::Assert(!function_return);
    function_return = signature.contained.get();
    auto body = SCompound();
    function_return = nullptr;
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