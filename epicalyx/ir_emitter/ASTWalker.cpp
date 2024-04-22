#include "ASTWalker.h"
#include "Default.h"
#include "Decltype.h"

namespace epi {

using namespace ast;

ASTWalker::BinopCastResult ASTWalker::BinopCastHelper(var_index_t left, var_index_t right) {
  auto left_v = emitter.vars[left];
  auto right_v = emitter.vars[right];
  auto left_t = left_v.type;
  auto right_t = right_v.type;
  if (left_t == right_t) {
    return {left_t, left, right};
  }

  if (left_t == Emitter::Var::Type::Pointer) {
    switch (right_t) {
      case Emitter::Var::Type::I32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i32>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::U32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u32>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::I64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i64>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::U64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u64>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::Float:
      case Emitter::Var::Type::Double:
        throw std::runtime_error("Invalid operands for binop: pointer and floating point type");
      case Emitter::Var::Type::Pointer:
        // should have been hit before
        throw std::runtime_error("Unreachable");
      default: throw std::runtime_error("Bad binop");
    }
  }

  if (right_t == Emitter::Var::Type::Pointer) {
    switch (left_t) {
      case Emitter::Var::Type::I32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i32>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::U32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u32>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::I64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i64>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::U64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u64>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::Float:
      case Emitter::Var::Type::Double:
        throw std::runtime_error("Invalid operands for binop: floating point type and pointer");
      default: throw std::runtime_error("Bad binop");
    }
  }

  switch (left_t) {
    case Emitter::Var::Type::I32: {
      switch (right_t) {
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::U32: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ left_t }, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::I64: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ left_t }, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ left_t }, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::U64: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({right_t}, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({right_t}, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::Float: {
      switch (right_t) {
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({right_t}, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({left_t}, right);
          return {{left_t}, left, current};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::Double: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({left_t}, right);
          return {{left_t}, left, current};
        default: throw std::runtime_error("Bad binop");
      }
    }
    default: throw std::runtime_error("Bad binop");
  }
}

}