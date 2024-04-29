#include "Tokenizer.h"
#include "Token.h"
#include "Default.h"
#include "Identifier.h"
#include "SStream.h"
#include "Containers.h"

#include <array>

namespace epi {

extern const cotyl::unordered_map<const std::string, TokenType> Punctuators;
extern const cotyl::unordered_map<const std::string, TokenType> Keywords;


template<typename T, typename ...Args>
AnyToken Tokenizer::Make(Args&&... args) {
  return T(std::forward<Args>(args)...);
}

void Tokenizer::SkipBlanks() {
  in_stream.SkipWhile(std::isspace);
}

bool Tokenizer::IsEOS() {
  SkipBlanks();
  return in_stream.EOS();
}

AnyToken Tokenizer::GetNew() {
  char c;
  SkipBlanks();
  if (!in_stream.Peek(c)) {
    throw cotyl::EndOfFileException();
  }

  if (detail::is_valid_ident_start(c)) {
    // identifier, keyword or prefixed character sequence
    if (in_stream.IsAfter(0, 'L', 'u', 'U')) {
      if (in_stream.IsAfter(1, '\"')) {
        // prefixed string literal
        in_stream.Skip();
        return Make<StringConstantToken>(ReadString('\"'));
      }
      else if (in_stream.IsAfter(1, '8') && in_stream.IsAfter(2, '\"')) {
        in_stream.Skip(2);
        return Make<StringConstantToken>(ReadString('\"'));
      }
      else if (in_stream.IsAfter(1, '\'')) {
        // prefixed char literal
        bool is_unsigned = in_stream.IsAfter(0, 'u', 'U');
        auto char_string = ReadString('\'');
        u32 value = 0;
        for (auto k : char_string) {
          value <<= 8;
          value |= (u8)k;
        }

        if (is_unsigned) {
          return Make<NumericalConstantToken<u32>>(value);
        }
        return Make<NumericalConstantToken<i32>>((i32)value);
      }
    }

    // identifier or keyword
    auto identifier = detail::get_identifier(in_stream);
    if (Keywords.contains(identifier)) {
      return Make<KeywordToken>(Keywords.at(identifier));
    }
    else {
      return Make<IdentifierToken>(cotyl::CString{std::move(identifier)});
    }
  }
  else if (std::isdigit(c) || (c == '.' && in_stream.PredicateAfter(1, std::isdigit))) {
    // numerical constant
    return ReadNumericalConstant();
  }
  else if (c == '"') {
    // string literal
    return Make<StringConstantToken>(ReadString('\"'));
  }
  else if (c == '\'') {
    // char literal
    auto char_string = ReadString('\'');
    u32 value = 0;
    for (auto k : char_string) {
      value <<= 8;
      value |= (u8)k;
    }
    return Make<NumericalConstantToken<i32>>((i32)value);
  }
  else {
    // punctuator
    if (in_stream.SequenceAfter(0, '.', '.', '.')) {
      in_stream.Skip(3);
      return Make<PunctuatorToken>(TokenType::Ellipsis);
    }

    cotyl::StringStream str{};
    do {
      str << c;
      if (Punctuators.contains(str.current())) {
        in_stream.Skip();
      }
      else {
        std::string punctuator = str.current();
        punctuator.erase(punctuator.size() - 1);
        return Make<PunctuatorToken>(Punctuators.at(punctuator));
      }
    } while (in_stream.Peek(c));

    if (Punctuators.contains(str.current())) {
      return Make<KeywordToken>(Punctuators.at(str.finalize()));
    }

    throw cotyl::EndOfFileException();
  }
}

static constexpr std::array<i32, 0x100> ASCIIHexToInt = {
        // ASCII
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

cotyl::CString Tokenizer::ReadString(const char delimiter) {
  cotyl::StringStream str{};

  in_stream.Eat(delimiter);
  char c;
  for (c = in_stream.Get(); c != delimiter; c = in_stream.Get()) {
    if (c == '\\') {
      // escape sequence
      c = in_stream.Get();
      switch (c) {
        case '\'': case '\"': case '\\': str << c; break;
        case 'a': str << '\a'; break;
        case 'b': str << '\b'; break;
        case 'f': str << '\f'; break;
        case 'n': str << '\n'; break;
        case 'r': str << '\r'; break;
        case 't': str << '\t'; break;
        case 'v': str << '\v'; break;
        case '?': str << '\?'; break;
        case 'x': {
          // hex literal
          char hex = 0;
          while (in_stream.PredicateAfter(0, std::isxdigit)) {
            c = in_stream.Get();
            hex <<= 4;
            hex |= ASCIIHexToInt[c];
          }
          str << hex;
          break;
        }
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': {
          // octal literal
          char oct = c;
          for (int count = 0; in_stream.PredicateAfter(0, std::isdigit) && count < 3; count++) {
            if (in_stream.Peek(c) && c < '8') {
              c = in_stream.Get();
              oct <<= 3;
              oct |= c - '0';
            }
          }
          str << oct;
          break;
        }
        default:
          throw cotyl::FormatExcept("Invalid escape sequence: %c", c);
      }
    }
    else {
      str << c;
    }
  }

  return str.cfinalize();
}

AnyToken Tokenizer::ReadNumericalConstant() {
  cotyl::StringStream value{};
  bool dot = false;
  bool exponent = false;
  bool octal = false;
  bool hex = false;
  bool is_float = false;

  if (in_stream.IsAfter(0, '0')) {
    if (in_stream.IsAfter(1, 'x', 'X')) {
      // hexadecimal
      hex = true;
      in_stream.Skip(2);
    }
    else {
      octal = true;
      in_stream.Skip();
    }
  }
  else if (in_stream.IsAfter(0, '.')) {
    dot = true;
    is_float = true;
    value << '0' << '.';
    in_stream.Skip();
  }

  while (true) {
    while ((hex && in_stream.PredicateAfter(0, std::isxdigit)) || (!hex && in_stream.PredicateAfter(0, std::isdigit))) {
      value << in_stream.Get();
    }

    if (in_stream.IsAfter(0, '.')) {
      if (hex) {
        throw std::runtime_error("No decimals allowed in hex float literal");
      }

      if (dot) {
        throw std::runtime_error("Invalid float literal: double decimal point");
      }

      value << in_stream.Get();
      dot = true;
      is_float = true;
    }
    else if (in_stream.IsAfter(0, 'e', 'E')) {
      if (exponent) {
        throw std::runtime_error("Invalid float literal: double exponent");
      }

      if (hex) {
        throw std::runtime_error("Invalid float literal: wrong exponent character for hex float");
      }

      value << in_stream.Get();

      // no more dots after exponent
      dot = true;
      exponent = true;
      is_float = true;

      if (in_stream.IsAfter(0, '+', '-')) {
        // exponent sign
        value << in_stream.Get();
      }
    }
    else if (in_stream.IsAfter(0, 'p', 'P')) {
      // hex float exponent
      if (!hex) {
        throw std::runtime_error("Invalid float literal: wrong exponent character for decimal float");
      }
      value << in_stream.Get();

      exponent = true;
      is_float = true;

      if (in_stream.IsAfter(0, '+', '-')) {
        // exponent sign
        value << in_stream.Get();
      }
    }
    else {
      // no special meaning, number is parsed
      break;
    }
  }

  int is_long = 0;
  bool is_unsigned = false;
  if (!octal && in_stream.IsAfter(0, 'f', 'F')) {
    in_stream.Skip();
    is_float = true;
  }
  else if (is_float && in_stream.IsAfter(0, 'l', 'L')) {
    in_stream.Skip();
    is_long = 1;
  }
  else if (!is_float) {
    if (in_stream.IsAfter(0, 'u', 'U')) {
      in_stream.Skip();
      is_unsigned = true;
      if (in_stream.IsAfter(0, 'l', 'L')) {
        in_stream.Skip();
        is_long = 1;
        if (in_stream.IsAfter(0, 'l', 'L')) {
          in_stream.Skip();
          is_long = 2;
        }
      }
    }
    else if (in_stream.IsAfter(0, 'l', 'L')) {
      in_stream.Skip();
      is_long = 1;
      if (in_stream.IsAfter(0, 'l', 'L')) {
        in_stream.Skip();
        is_long = 2;
      }

      if (in_stream.IsAfter(0, 'u', 'U')) {
        in_stream.Skip();
        is_unsigned = true;
      }
    }
  }

  if (is_float) {
    double val = std::stod(value.finalize());
    if (is_long || (val != (float)val)) {
      return Make<NumericalConstantToken<double>>(val);
    }
    else {
      return Make<NumericalConstantToken<float>>((float)val);
    }
  }
  if (is_unsigned) {
    u64 val = 0;
    if (octal) {
      for (auto c : value.finalize()) {
        if (c > '7') {
          throw std::runtime_error("Invalid octal constant");
        }
        val <<= 3;
        val |= c - '0';
      }
    }
    else if (hex) {
      val = std::stoull(value.finalize(), nullptr, 16);
    }
    else {
      val = std::stoull(value.finalize());
    }
    if (is_long == 2 || val > UINT32_MAX) {
      return Make<NumericalConstantToken<u64>>(val);
    }
    else {
      return Make<NumericalConstantToken<u32>>((u32)val);
    }
  }
  else {
    i64 val = 0;
    if (octal) {
      for (auto c : value.finalize()) {
        if (c > '7') {
          throw std::runtime_error("Invalid octal constant");
        }
        val <<= 3;
        val |= c - '0';
      }
    }
    else if (hex) {
      val = (i64)std::stoull(value.finalize(), nullptr, 16);
    }
    else {
      val = std::stoll(value.finalize());
    }

    if (is_long == 2 || val > INT32_MAX || val < INT32_MIN) {
      return Make<NumericalConstantToken<i64>>(val);
    }
    else {
      return Make<NumericalConstantToken<i32>>((i32)val);
    }
  }
}

}