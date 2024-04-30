#pragma once

#include "Default.h"

namespace epi::ast {

enum class StorageClass;
struct DeclarationNode;
struct FunctionDefinitionNode;

struct IdentifierNode;
template<typename T> struct NumericalConstantNode;
struct StringConstantNode;
struct ArrayAccessNode;
struct FunctionCallNode;
struct MemberAccessNode;
struct TypeInitializerNode;
struct PostFixNode;
struct UnopNode;
struct CastNode;
struct BinopNode;
struct TernaryNode;
struct AssignmentNode;
struct ExpressionListNode;

struct EmptyNode;
struct IfNode;
struct WhileNode;
struct DoWhileNode;
struct ForNode;
struct LabelNode;
struct SwitchNode;
struct CaseNode;
struct DefaultNode;
struct GotoNode;
struct ReturnNode;
struct BreakNode;
struct ContinueNode;
struct CompoundNode;

struct Node;
struct DeclNode;
struct StatNode;
struct ExprNode;

template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<ExprNode>;
using pStat = pNode<StatNode>;

struct NodeVisitor;

}