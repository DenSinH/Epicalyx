#pragma once

/*
 * Basic parser class to parse constant expressions
 * in the preprocessing stage. This is to prevent
 * having to allocate a lot of bloat (identifier maps,
 * typename maps, etc.) when parsing a preprocessor
 * constant expression
 * */

#include <iosfwd>         // for ostream
#include <string>         // for string
#include <utility>        // for move

#include "Default.h"      // for i64, u32
#include "Exceptions.h"   // for Exception
#include "Locatable.h"    // for Locatable
#include "ast/NodeFwd.h"  // for pExpr

namespace epi { namespace cotyl { struct CString; } }
namespace epi { namespace cotyl { template <typename T> struct Stream; } }

namespace epi {

struct AnyToken;
enum class TokenType : u32;


struct ParserError : cotyl::Exception {
  ParserError(std::string&& message) : 
      Exception("Parser Error", std::move(message)) { }
};


struct ExpressionParser : public cotyl::Locatable {

  ExpressionParser(cotyl::Stream<AnyToken>& in_stream);

  void PrintLoc(std::ostream& out) const override;
  i64 EConstexpr();

  // needs public access for shorthand parsing Binop Expressions
  ast::pExpr EBinopBaseVCall();
  template<ast::pExpr (ExpressionParser::*SubNode)(), TokenType... types>
  ast::pExpr EBinopImpl();

protected:
  virtual ast::pExpr ResolveIdentifier(cotyl::CString&& name) const = 0;
  virtual ast::pExpr EPrimary();
  virtual ast::pExpr EBinopBase();
  ast::pExpr EBinop();
  ast::pExpr ETernary();
  virtual ast::pExpr EAssignment();
  ast::pExpr EExpression();
  ast::pExpr EExpressionList();

  cotyl::Stream<AnyToken>& in_stream;
};

}
