#include "Tokenizer.h"

#include <stdint.h>                                   // for INT32_MAX, UINT...
#include <cctype>                                     // for isdigit, isspace
#include <string>                                     // for stoull, string
#include <utility>                                    // for move, forward

#include "CString.h"                                  // for CString
#include "Containers.h"                               // for unordered_map, operator!=
#include "Default.h"                                  // for i32, u32, i64, u64
#include "Escape.h"                                   // for Unescape
#include "Exceptions.h"                               // for Exception
#include "Format.h"                                   // for FormatExcept
#include "Identifier.h"                               // for get_identifier
#include "SStream.h"                                  // for StringStream
#include "Token.h"                                    // for NumericalConsta...
#include "TokenType.h"                                // for TokenType


namespace epi {

extern const cotyl::unordered_map<const std::string, TokenType> Punctuators;
extern const cotyl::unordered_map<const std::string, TokenType> Keywords;

struct TokenizerError : cotyl::Exception {
  TokenizerError(std::string&& message) : 
    cotyl::Exception("Tokenizer Error", std::move(message)) { }
};


template<typename T, typename ...Args>
AnyToken Tokenizer::Make(Args&&... args) {
  return T(std::forward<Args>(args)...);
}

void Tokenizer::SkipBlanks() {
  in_stream.SkipWhile(isspace);
}

bool Tokenizer::IsEOS() {
  SkipBlanks();
  return in_stream.EOS();
}

AnyToken Tokenizer::GetNew() {
  char c;
  SkipBlanks();
  if (!in_stream.Peek(c)) {
    throw cotyl::EOSError();
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
        bool is_long = in_stream.IsAfter(0, 'L');
        in_stream.Skip();
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
    auto identifier = detail::get_identifier(in_stream).str();
    if (Keywords.contains(identifier)) {
      return Make<KeywordToken>(Keywords.at(identifier));
    }
    else {
      return Make<IdentifierToken>(cotyl::CString{std::move(identifier)});
    }
  }
  else if (std::isdigit(c) || (c == '.' && in_stream.PredicateAfter(1, isdigit))) {
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

    throw cotyl::EOSError();
  }
}

cotyl::CString Tokenizer::ReadString(char _delimiter) {
  char delimiter;
  if (_delimiter) {
    in_stream.Eat(_delimiter);
    delimiter = _delimiter;
  }
  else {
    // automatic deduction
    delimiter = in_stream.Get();
    if (delimiter == '<') {
      delimiter = '>';
    }
    // todo: should I allow '\'' here for completeness?
    else if (delimiter != '\"') {
      throw cotyl::FormatExcept<TokenizerError>("Unexpected string delimiter: %c", delimiter);
    }
  }

  cotyl::StringStream str{};
  char c;
  for (c = in_stream.Get(); c != delimiter; c = in_stream.Get()) {
    if (c == '\\') cotyl::Unescape(str, in_stream);
    else str << c;
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
    value << '0';
    in_stream.Skip();
    if (in_stream.IsAfter(0, 'x', 'X')) {
      // hexadecimal
      hex = true;
      in_stream.Skip();
    }
    else {
      octal = true;
    }
  }
  else if (in_stream.IsAfter(0, '.')) {
    dot = true;
    is_float = true;
    value << '0' << '.';
    in_stream.Skip();
  }

  while (true) {
    while ((hex && in_stream.PredicateAfter(0, isxdigit)) || (!hex && in_stream.PredicateAfter(0, isdigit))) {
      value << in_stream.Get();
    }

    if (in_stream.IsAfter(0, '.')) {
      if (hex) {
        throw TokenizerError("No decimals allowed in hex float literal");
      }

      if (dot) {
        throw TokenizerError("Invalid float literal: double decimal point");
      }

      value << in_stream.Get();
      dot = true;
      is_float = true;
    }
    else if (in_stream.IsAfter(0, 'e', 'E')) {
      if (exponent) {
        throw TokenizerError("Invalid float literal: double exponent");
      }

      if (hex) {
        throw TokenizerError("Invalid float literal: wrong exponent character for hex float");
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
        throw TokenizerError("Invalid float literal: wrong exponent character for decimal float");
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

  // see ISO/IEC 9899:201x 6.4.4.1 for integral constant types
  // basically:
  // if unsigned is specified: try uint, then ulong, then ulonglong
  // otherwise: if hex / oct, try int, uint, long, ulong, lonlong, ulonlong
  //            if dec, try int, long, long long
  if (is_unsigned) {
    u64 val = 0;
    if (octal) {
      val = std::stoull(value.finalize(), nullptr, 8);
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
    if (octal || hex) {
      u64 val;
      if (octal) {
        val = std::stoull(value.finalize(), nullptr, 8);
      }
      else {
        val = std::stoull(value.finalize(), nullptr, 16);
      }
      if (is_long < 2) {
        if (val <= INT32_MAX) {
          return Make<NumericalConstantToken<i32>>((i32)val);
        }
        else if (val <= UINT32_MAX) {
          return Make<NumericalConstantToken<u32>>((u32)val);
        }
      }
      if (val <= INT64_MAX) {
        return Make<NumericalConstantToken<i64>>((i64)val);
      }
      return Make<NumericalConstantToken<u64>>(val);
    }
    else {
      i64 val = stoll(value.finalize());
      if (is_long == 2 || val >= INT32_MAX || val <= INT32_MIN) {
        return Make<NumericalConstantToken<i64>>(val);
      }
      else {
        return Make<NumericalConstantToken<i32>>((i32)val);
      }
    }
  }
}

}