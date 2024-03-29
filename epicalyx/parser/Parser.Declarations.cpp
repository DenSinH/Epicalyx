#include "Parser.h"
#include "Is.h"
#include "Cast.h"
#include "ast/Declaration.h"
#include "ast/Statement.h"

#include <optional>
#include <iostream>


namespace epi {

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
      return typedefs.Has(static_cast<const tIdentifier*>(current)->name);
    }
    default:
      return false;
  }
}

void Parser::DStaticAssert() {
  in_stream.EatSequence(TokenType::StaticAssert, TokenType::LParen);
  auto expr = Parser::EConstexpr();
  in_stream.Eat(TokenType::Comma);
  in_stream.Expect(TokenType::StringConstant);
  auto str = cotyl::unique_ptr_cast<tStringConstant>(in_stream.Get())->value;
  in_stream.EatSequence(TokenType::RParen, TokenType::SemiColon);

  if (!expr) {
    throw cotyl::FormatExceptStr("Static assertion failed: %s", str);
  }
}

pType<> Parser::DEnum() {
  in_stream.Eat(TokenType::Enum);
  std::string name;
  if (in_stream.IsAfter(0, TokenType::Identifier)) {
    // enum name
    name = cotyl::unique_ptr_cast<tIdentifier>(in_stream.Get())->name;
    if (!in_stream.EatIf(TokenType::LBrace)) {
      // check if enum was defined before
      if (!enums.Has(name)) {
        throw cotyl::FormatExceptStr("Undefined enum %s", name);
      }
      return MakeType<ValueType<enum_type>>(CType::LValueNess::None);
    }
  }
  else {
    in_stream.Eat(TokenType::LBrace);
  }
  // enum <name> { ... }

  enum_type counter = 0;
  do {
    in_stream.Expect(TokenType::Identifier);
    std::string constant = cotyl::unique_ptr_cast<tIdentifier>(in_stream.Get())->name;
    if (in_stream.EatIf(TokenType::Assign)) {
      // constant = value
      // update counter
      counter = EConstexpr();
    }
    enum_values.Set(constant, counter++);
    variables.Set(constant, MakeType<ValueType<enum_type>>(counter, CType::LValueNess::None, CType::Qualifier::Const));
    if (!in_stream.EatIf(TokenType::Comma)) {
      // no comma: expect end of enum declaration
      in_stream.Eat(TokenType::RBrace);
      break;
    }
  } while(!in_stream.EatIf(TokenType::RBrace));

  enums.Add(name);
  return MakeType<ValueType<enum_type>>(CType::LValueNess::None);
}

pType<> Parser::DStruct() {
  pType<StructUnionType> type;
  std::string name;
  bool is_struct = true;
  if (!in_stream.EatIf(TokenType::Struct)) {
    in_stream.Eat(TokenType::Union);
    is_struct = false;
  }

  if (in_stream.IsAfter(0, TokenType::Identifier)) {
    name = cotyl::unique_ptr_cast<tIdentifier>(in_stream.Get())->name;
    if (!in_stream.EatIf(TokenType::LBrace)) {
      if (is_struct) {
        return structdefs.Get(name)->Clone();
      }
      else {
        return uniondefs.Get(name)->Clone();
      }
    }
  }
  else {
    in_stream.Eat(TokenType::LBrace);
  }

  if (is_struct) {
    type = MakeType<StructType>(name, CType::LValueNess::Assignable);
  }
  else {
    type = MakeType<UnionType>(name, CType::LValueNess::Assignable);
  }

  while(!in_stream.EatIf(TokenType::RBrace)) {
    if (in_stream.IsAfter(0, TokenType::StaticAssert)) {
      DStaticAssert();
    }
    else {
      auto ctype = DSpecifier();

      do {
        pNode<Declaration> decl = DDeclarator(ctype.first, ctype.second);
        size_t size = 0;
        if (decl->storage != StorageClass::None) {
          throw std::runtime_error("Invalid storage class specifier in struct definition");
        }
        if (in_stream.EatIf(TokenType::Colon)) {
          size = EConstexpr();
        }
        type->AddField(decl->name, size, decl->type->Clone());
      } while (in_stream.EatIf(TokenType::Comma));
      in_stream.Eat(TokenType::SemiColon);
    }
  }

  if (!name.empty()) {
    if (is_struct) {
      structdefs.Set(name, type);
    }
    else {
      uniondefs.Set(name, type);
    }
  }
  return type;
}

std::pair<pType<>, StorageClass> Parser::DSpecifier() {
  std::optional<StorageClass> storage{};
  enum class Type {
    Void, Char, Short, Long, LongLong, Int, ShortInt, LongInt, LongLongInt, Float, Double
  };

  pType<> ctype;

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
    if (ctype) {
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
        ctype = DStruct();
        break;
      }
      case TokenType::Enum: {
        assert_no_type();
        assert_no_sign();
        assert_no_ctype();
        ctype = DEnum();
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

      case TokenType::Const:    in_stream.Skip(); qualifiers |= CType::Qualifier::Const; break;
      case TokenType::Restrict: in_stream.Skip(); qualifiers |= CType::Qualifier::Restrict; break;
      case TokenType::Volatile: in_stream.Skip(); qualifiers |= CType::Qualifier::Volatile; break;
      case TokenType::Atomic: {
        in_stream.Skip();
        if (in_stream.IsAfter(1, TokenType::LParen)) {
          throw cotyl::UnimplementedException("_Atomic");
        }
        qualifiers |= CType::Qualifier::Atomic;
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
        std::string ident_name = static_cast<const tIdentifier*>(current)->name;
        if (typedefs.Has(ident_name)) {
          if (ctype) {
            throw std::runtime_error("Bad declaration");
          }
          in_stream.Skip();
          ctype = typedefs.Get(ident_name)->Clone();
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

  auto make = [=]<typename T>() -> pType<> {
    if (_sign == -1) return MakeType<ValueType<T>>(CType::LValueNess::Assignable, qualifiers);
    return MakeType<ValueType<std::make_unsigned_t<T>>>(CType::LValueNess::Assignable, qualifiers);
  };

  if (!type.has_value()) {
    if (!ctype) {
      // ctype might have been set by a typedef name
      ctype = make.operator()<i32>();
    }
  }
  else {
    if (ctype) {
      // ctype might have been set by a typedef name
      throw std::runtime_error("Invalid type specifier");
    }

    switch (type.value()) {
      case Type::Int: case Type::LongInt: case Type::Long:
        ctype = make.operator()<i32>(); break;
      case Type::Short: case Type::ShortInt:
        ctype = make.operator()<i16>(); break;
      case Type::LongLong: case Type::LongLongInt:
        ctype = make.operator()<i64>(); break;
      case Type::Char:
        ctype = make.operator()<i8>(); break;
      case Type::Void:
        ctype = MakeType<VoidType>(qualifiers); break;
      case Type::Float:
        ctype = MakeType<ValueType<float>>(CType::LValueNess::Assignable, qualifiers); break;
      case Type::Double:
        ctype = MakeType<ValueType<double>>(CType::LValueNess::Assignable, qualifiers); break;
    }
  }
  return std::make_pair(ctype, storage ? storage.value() : StorageClass::None);
}

std::string Parser::DDirectDeclaratorImpl(std::stack<pType<PointerType>>& dest) {
  std::string name;
  pType<PointerType> ctype;

  const Token* current;
  while (true) {
    in_stream.Peek(current);
    // nested (abstract) declarators
    switch (current->type) {
      case TokenType::Asterisk: {
        // pointer with qualifiers
        in_stream.Skip();
        u32 ptr_qualifiers = 0;
        while (in_stream.IsAfter(0, TokenType::Const, TokenType::Restrict, TokenType::Volatile, TokenType::Atomic)) {
          switch (in_stream.Get()->type) {
            case TokenType::Const: ptr_qualifiers |= CType::Qualifier::Const; break;
            case TokenType::Restrict: ptr_qualifiers |= CType::Qualifier::Restrict; break;
            case TokenType::Volatile: ptr_qualifiers |= CType::Qualifier::Volatile; break;
            case TokenType::Atomic: ptr_qualifiers |= CType::Qualifier::Atomic; break;
            default:
              // [[unreachable]]
              break;
          }
        }
        ctype = MakeType<PointerType>(std::move(ctype), CType::LValueNess::Assignable, ptr_qualifiers);
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
            ctype = MakeType<FunctionType>(std::move(ctype), false, CType::LValueNess::Assignable);
            in_stream.Skip();
            break;
          }
          case TokenType::LParen: {
            // ((direct-declarator))
            in_stream.Skip();
            std::string _name = DDirectDeclaratorImpl(dest);
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
            std::string _name = DDirectDeclaratorImpl(dest);
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
            std::string ident_name = cotyl::unique_ptr_cast<tIdentifier>(in_stream.Get())->name;
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
            auto type = MakeType<FunctionType>(std::move(ctype), false, CType::LValueNess::Assignable);
            do {
              auto arg_specifier = DSpecifier();
              if (arg_specifier.second != StorageClass::None) {
                throw std::runtime_error("Bad storage specifier on function argument");
              }

              auto arg = DDeclarator(arg_specifier.first, StorageClass::Auto);
              type->AddArg(arg->name, arg->type);
              if (in_stream.EatIf(TokenType::Comma)) {
                if (in_stream.EatIf(TokenType::Ellipsis)) {
                  type->variadic = true;
                  in_stream.Eat(TokenType::RParen);
                  break;
                }
              }
              else {
                in_stream.Eat(TokenType::RParen);
                break;
              }
            } while (true);
            ctype = type;
          }
        }
        break;
      }
      case TokenType::Identifier: {
        if (!name.empty()) {
          throw std::runtime_error("Double name in declaration");
        }
        name = cotyl::unique_ptr_cast<tIdentifier>(in_stream.Get())->name;
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
        ctype = MakeType<ArrayType>(std::move(ctype), size);
        break;
      }
      default: {
        if (ctype) {
          // ctype might be empty
          // for example: int (a) = 0;
          dest.push(std::move(ctype));
        }
        return name;
      }
    }
  }
}

pNode<Declaration> Parser::DDeclarator(pType<> ctype, StorageClass storage) {
  std::string name;
  std::stack<pType<PointerType>> direct{};

  name = DDirectDeclaratorImpl(direct);
  while (!direct.empty()) {
    // weird reverse-nested declarators...
    pType<PointerType> ptr = direct.top();
    direct.pop();

    // might already contain nested type
    pType<PointerType> p = ptr;
    while (p->contained) {
      p = std::dynamic_pointer_cast<PointerType>(p->contained);
    }
    p->contained = ctype;
    ctype = std::move(ptr);
  }
  return std::make_unique<Declaration>(ctype, name, storage);
}

void Parser::DInitDeclaratorList(std::vector<pNode<Declaration>>& dest) {
  auto ctype = DSpecifier();

  do {
    pNode<Declaration> decl = DDeclarator(ctype.first, ctype.second);
    if (decl->storage == StorageClass::Typedef) {
      // store typedef names
      if (decl->name.empty()) {
        throw std::runtime_error("Typedef declaration must have a name");
      }
      typedefs.Set(decl->name, decl->type);
    }
    else {
      decl->VerifyAndRecord(*this);
      if (in_stream.EatIf(TokenType::Assign)) {
        // type var = <expression> or {initializer list}
        if (decl->name.empty()) {
          throw std::runtime_error("Cannot assign to nameless variable");
        }
        decl->value = EInitializer();
        dest.push_back(std::move(decl));
      }
      else {
        // type var, var2, var3
        if (!decl->name.empty()) {
          dest.push_back(std::move(decl));
        }
        else {
          // warn: statement has no effect
        }
      }
    }
  } while (in_stream.EatIf(TokenType::Comma));
}

pNode<FunctionDefinition> Parser::ExternalDeclaration(std::vector<pNode<Declaration>>& dest) {
  if (in_stream.IsAfter(0, TokenType::StaticAssert)) {
    DStaticAssert();
    return nullptr;
  }
  auto ctype = DSpecifier();
  pNode<Declaration> decl = DDeclarator(ctype.first, ctype.second);

  // signature { body }
  if (in_stream.EatIf(TokenType::LBrace)) {
    if (!decl->type->IsFunction()) {
      throw std::runtime_error("Unexpected compound statement after external declaration");
    }
    pType<FunctionType> signature = std::static_pointer_cast<FunctionType>(decl->type->Clone());
    signature->lvalue = CType::LValueNess::None;

    std::string symbol = decl->name;
    if (symbol.empty()) {
      throw std::runtime_error("Missing name in function declaration");
    }

    variables.NewLayer();
    for (const auto& arg : signature->arg_types) {
      if (arg.name.empty()) {
        throw std::runtime_error("Nameless argument in function definition");
      }
      variables.Set(arg.name, arg.type);
    }
    function_return = signature->contained;
    auto body = SCompound();
    function_return = nullptr;
    variables.PopLayer();
    in_stream.Eat(TokenType::RBrace);
    return std::make_unique<FunctionDefinition>(signature, symbol, std::move(body));
  }

  // normal declaration
  do {
    if (decl->storage == StorageClass::Typedef) {
      // store typedef names
      if (decl->name.empty()) {
        throw std::runtime_error("Typedef declaration must have a name");
      }
      typedefs.Set(decl->name, decl->type);
    }
    else {
      if (in_stream.EatIf(TokenType::Assign)) {
        // type var = <expression> or {initializer list}
        decl->value = EInitializer();
        dest.push_back(std::move(decl));
      }
      else {
        // type var, var2, var3
        dest.push_back(std::move(decl));
      }
    }
    if (in_stream.EatIf(TokenType::SemiColon)) {
      return nullptr;
    }
    in_stream.Eat(TokenType::Comma);
    decl = DDeclarator(ctype.first, ctype.second);
  } while (true);
}

}