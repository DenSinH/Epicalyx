#include "Parser.h"
#include "Is.h"
#include "nodes/Declaration.h"

#include <optional>
#include <iostream>


namespace epi {

bool Parser::IsDeclarationSpecifier(int after) {
  pToken current;
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
      return typedefs.Has(std::dynamic_pointer_cast<tIdentifier>(current)->name);
    }
    default:
      return false;
  }
}

void Parser::DStaticAssert() {
  in_stream.EatSequence(TokenType::StaticAssert, TokenType::LParen);
  auto expr = Parser::ETernary();
  in_stream.EatSequence(TokenType::RParen, TokenType::SemiColon);

  // todo: consteval expression
}

pType<> Parser::DEnum() {
  in_stream.Eat(TokenType::Enum);
  std::string name;
  if (in_stream.IsAfter(0, TokenType::Identifier)) {
    // enum name
    name = std::dynamic_pointer_cast<tIdentifier>(in_stream.Get())->name;
    if (!in_stream.EatIf(TokenType::LBrace)) {
      // check if enum was defined before
      if (!enums.Has(name)) {
        throw cotyl::FormatExceptStr("Undefined enum %s", name);
      }
      return MakeType<ValueType<enum_type>>();
    }
  }
  else {
    in_stream.Eat(TokenType::LBrace);
  }
  // enum <name> { ... }

  i64 counter = 0;
  do {
    in_stream.Expect(TokenType::Identifier);
    std::string constant = std::dynamic_pointer_cast<tIdentifier>(in_stream.Get())->name;
    if (in_stream.EatIf(TokenType::Assign)) {
      // constant = value
      // update counter
      counter = EConstexpr();
    }
    enum_values.Set(constant, counter++);
    if (!in_stream.EatIf(TokenType::Comma)) {
      // no comma: expect end of enum declaration
      in_stream.Eat(TokenType::RBrace);
      break;
    }
  } while(!in_stream.EatIf(TokenType::RBrace));

  enums.Set(name, true);
  return MakeType<ValueType<enum_type>>();
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
    name = std::dynamic_pointer_cast<tIdentifier>(in_stream.Get())->name;
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
    type = MakeType<StructType>(name);
  }
  else {
    type = MakeType<UnionType>(name);
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
        if (decl->storage != StorageClass::Auto) {
          // todo: don't allow storage class specifiers
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

  pToken current;

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
          throw std::runtime_error("Unimplemented type: long double");
        }
        type = Type::Double;
        break;
      }
      case TokenType::Bool: {
        in_stream.Skip();
        throw std::runtime_error("Unimplemented type: _Bool");
      }
      case TokenType::Complex: {
        in_stream.Skip();
        throw std::runtime_error("Unimplemented type: _Complex");
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
          throw std::runtime_error("Unimplemented specifier: _Atomic");
        }
        qualifiers |= CType::Qualifier::Atomic;
        break;
      }

      case TokenType::Inline: {
        in_stream.Skip();
        // todo
        is_function = true;
        break;
      }
      case TokenType::Noreturn: {
        in_stream.Skip();
        // todo
        is_function = true;
        break;
      }

      case TokenType::Alignas: {
        in_stream.Skip();
        throw std::runtime_error("Unimplemented specifier: _Alignas");
      }

      case TokenType::Identifier: {
        std::string ident_name = std::dynamic_pointer_cast<tIdentifier>(current)->name;
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
    if (_sign == -1) return MakeType<ValueType<T>>(CType::LValueNess::None, qualifiers);
    return MakeType<ValueType<std::make_unsigned_t<T>>>(CType::LValueNess::None, qualifiers);
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
        ctype = MakeType<ValueType<float>>(CType::LValueNess::None, qualifiers); break;
      case Type::Double:
        ctype = MakeType<ValueType<double>>(CType::LValueNess::None, qualifiers); break;
    }
  }
  return std::make_pair(ctype, storage ? storage.value() : StorageClass::Auto);
}

std::string Parser::DDirectDeclaratorImpl(std::stack<pType<PointerType>>& dest) {

  std::string name;
  pType<PointerType> ctype;

  pToken current;
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
        ctype = MakeType<PointerType>(std::move(ctype), CType::LValueNess::None, ptr_qualifiers);
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
            ctype = MakeType<FunctionType>(std::move(ctype));
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
            std::string ident_name = std::dynamic_pointer_cast<tIdentifier>(in_stream.Get())->name;
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
            auto type = MakeType<FunctionType>(std::move(ctype));
            do {
              auto arg_specifier = DSpecifier();
              if (arg_specifier.second != StorageClass::Auto) {
                throw std::runtime_error("Bad storage specifier on function argument");
              }

              type->AddArg(DDeclarator(arg_specifier.first, StorageClass::Auto)->type);
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
        name = std::dynamic_pointer_cast<tIdentifier>(in_stream.Get())->name;
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
        ctype = MakeType<ArrayType>(std::move(ctype), 0);
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


void Parser::DInitDeclaratorList(std::vector<pNode<InitDeclaration>>& dest) {
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
      if (in_stream.EatIf(TokenType::Assign)) {
        // type var = <expression> or {initializer list}
        dest.push_back(std::make_unique<InitDeclaration>(std::move(decl), EInitializer()));
      }
      else {
        // type var, var2, var3
        dest.push_back(std::make_unique<InitDeclaration>(std::move(decl)));
      }
    }
  } while (in_stream.EatIf(TokenType::Comma));
}

}