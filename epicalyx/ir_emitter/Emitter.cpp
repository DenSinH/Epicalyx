#include "Emitter.h"
#include "ASTWalker.h"
#include "ast/Node.h"
#include "ast/Declaration.h"


namespace epi {

Emitter::Emitter() {

}

block_label_t Emitter::MakeBlock() {
  // block 0 is special
  return current_function->AddBlock().first;
}

void Emitter::SelectBlock(block_label_t id) {
  cotyl::Assert(current_function->blocks.at(id).empty(), "Expected empty block on selection");
  reachable = true;
  current_block = id;
}

void Emitter::SetFunction(calyx::Function& func) {
  current_function = &func;
  const auto entry = MakeBlock();
  cotyl::Assert(entry == calyx::Function::Entry, "Expected function entry to be block 1");
  SelectBlock(entry);
  ir_counter = 1;
  c_counter = 1;
  
  // first var is special
  vars = {{Var::Type::I32}};
}

void Emitter::NewFunction(cotyl::CString&& symbol) {
  auto func = calyx::Function{std::move(symbol)};
  const auto& sym = func.symbol;
  SetFunction(program.functions.emplace(sym, std::move(func)).first->second);
}

void Emitter::MakeProgram(cotyl::vector<ast::DeclarationNode>& decls, cotyl::vector<ast::FunctionDefinitionNode>& funcs) {
  auto walker = ASTWalker(*this);
  for (auto& decl : decls) {
    walker.Visit(decl);
  }
  for (auto& func : funcs) {
    walker.Visit(func);
  }
}

}