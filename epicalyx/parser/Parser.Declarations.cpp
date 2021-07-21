#include "Parser.h"
#include "Is.h"
#include "nodes/Declarations.h"

#include <optional>
#include <iostream>


namespace epi {

void Parser::DStaticAssert() {
  in_stream.EatSequence(TokenType::StaticAssert, TokenType::LParen);
  auto expr = Parser::ETernary();
  in_stream.EatSequence(TokenType::RParen, TokenType::SemiColon);

  // todo: consteval expression
}

pType<> Parser::DSpecifier() {
  std::optional<StorageClass> storage{};
  enum class Type {
    Void, Char, Short, Long, LongLong, Int, ShortInt, LongInt, LongLongInt, Float, Double, StructUnionEnum
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

  auto assert_no_sign = [&sign]() {
    if (sign.has_value()) {
      throw std::runtime_error("Double type specifier in declaration");
    }
  };

  pToken current;

  do {
    current = in_stream.ForcePeek();
    bool was_specifier = true;

    switch (current->type) {
      case TokenType::Typedef:     set_storage(StorageClass::Typedef); break;
      case TokenType::Extern:      set_storage(StorageClass::Extern); break;
      case TokenType::Static:      set_storage(StorageClass::Static); break;
      case TokenType::ThreadLocal: set_storage(StorageClass::ThreadLocal); break;
      case TokenType::Auto:        set_storage(StorageClass::Auto); break;
      case TokenType::Register:    set_storage(StorageClass::Register); break;

      case TokenType::Void: {
        assert_no_type();
        assert_no_sign();
        type = Type::Void;
        break;
      }
      case TokenType::Char: {
        assert_no_type();
        type = Type::Char;
        break;
      }
      case TokenType::Short: {
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
        if (type.has_value()) {
          switch (type.value()) {
            case Type::Short: type = Type::ShortInt; break;
            case Type::Long: type = Type::LongInt; break;
            case Type::LongLong: type = Type::LongLongInt; break;
            default:
              assert_no_type();
          }
        }
        type = Type::Int;
        break;
      }
      case TokenType::Float: {
        assert_no_type();
        type = Type::Float;
        break;
      }
      case TokenType::Double: {
        if (type.has_value() && type.value() == Type::Long) {
          throw std::runtime_error("Unimplemented type: long double");
        }
        type = Type::Double;
        break;
      }
      case TokenType::Bool: {
        throw std::runtime_error("Unimplemented type: _Bool");
      }
      case TokenType::Complex: {
        throw std::runtime_error("Unimplemented type: _Complex");
      }

      case TokenType::Struct: {
        assert_no_type();
        assert_no_sign();
        type = Type::StructUnionEnum;
        // todo:
        break;
      }
      case TokenType::Union: {
        assert_no_type();
        assert_no_sign();
        type = Type::StructUnionEnum;
        // todo:
        break;
      }
      case TokenType::Enum: {
        assert_no_type();
        assert_no_sign();
        type = Type::StructUnionEnum;
        // todo:
        break;
      }

      case TokenType::Signed: {
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

      case TokenType::Const: qualifiers |= CType::Qualifier::Const; break;
      case TokenType::Restrict: qualifiers |= CType::Qualifier::Restrict; break;
      case TokenType::Volatile: qualifiers |= CType::Qualifier::Volatile; break;
      case TokenType::Atomic: {
        if (in_stream.IsAfter(1, TokenType::LParen)) {
          throw std::runtime_error("Unimplemented specifier: _Atomic");
        }
        qualifiers |= CType::Qualifier::Atomic;
        break;
      }

      case TokenType::Inline: {
        // todo
        is_function = true;
        break;
      }
      case TokenType::Noreturn: {
        // todo
        is_function = true;
        break;
      }

      case TokenType::Alignas: {
        throw std::runtime_error("Unimplemented specifier: _Alignas");
      }

      case TokenType::Identifier: {
        // todo: check if typedef name exists
        // if typedef name: type = blabla
        // in both cases, specifiers may come after;
        was_specifier = false;
        break;
      }

      default: {
        was_specifier = false;
        break;
      }
    }

    if (!was_specifier) break;
    in_stream.Skip();
  } while (true);

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
      case Type::StructUnionEnum: {
        throw std::runtime_error("Unimplemented: struct / unions / enums");
      }
    }
  }
  return ctype;
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
//              [[fallthrough]];
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
            // if not typdef name: direct declarator name
            if (!name.empty()) {
              throw std::runtime_error("Double name in declaration");
            }
            name = std::dynamic_pointer_cast<tIdentifier>(in_stream.Get())->name;
            break;

            // otherwise: fallthrough
          }
          default: {
            function_call:
            // has to be a function declaration with at least one parameter
            auto type = MakeType<FunctionType>(std::move(ctype));
            do {
              type->AddArg(DDeclarator()->type);
              if (in_stream.IsAfter(0, TokenType::Comma)) {
                in_stream.Skip();
              }
              else if (in_stream.IsAfter(0, TokenType::Ellipsis)) {
                type->variadic = true;
                in_stream.EatSequence(TokenType::Ellipsis, TokenType::RParen);
              }
              else {
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
        pType<> size = nullptr;
        while (in_stream.Peek(current) && current->type != TokenType::RBracket) {
          if (in_stream.IsAfter(0, TokenType::Const, TokenType::Restrict, TokenType::Volatile, TokenType::Atomic, TokenType::Static)) {
            in_stream.Skip();  // todo
          }
          else {
            // todo: size consteval
            // size = EAssignment();
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

pNode<Declarator> Parser::DDeclarator() {
  pType<> ctype = DSpecifier();
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
  return std::make_unique<Declarator>(ctype, name);
}

}