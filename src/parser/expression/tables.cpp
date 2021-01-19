#include "assign_expression.h"
#include "binop_expression.h"

const std::map<AssignmentExpression::AssignOp, std::string> AssignmentExpression::OpString = {
        {  AssignmentExpression::AssignOp::Eq, "=" },
        {  AssignmentExpression::AssignOp::MulEq, "*=" },
        {  AssignmentExpression::AssignOp::DivEq, "/=" },
        {  AssignmentExpression::AssignOp::ModEq, "%=" },
        {  AssignmentExpression::AssignOp::AddEq, "+=" },
        {  AssignmentExpression::AssignOp::SubEq, "-=" },
        {  AssignmentExpression::AssignOp::LShiftEq, "<<=" },
        {  AssignmentExpression::AssignOp::RShiftEq, ">>=" },
        {  AssignmentExpression::AssignOp::AndEq, "&=" },
        {  AssignmentExpression::AssignOp::XorEq, "^=" },
        {  AssignmentExpression::AssignOp::OrEq, "|=" },
};

const std::map<enum TokenType, AssignmentExpression::AssignOp> AssignmentExpression::TokenMap = {
        { TokenType::Assign, AssignOp::Eq },
        { TokenType::IMul, AssignOp::MulEq },
        { TokenType::IDiv, AssignOp::DivEq },
        { TokenType::IMod, AssignOp::ModEq },
        { TokenType::IPlus, AssignOp::AddEq },
        { TokenType::IMinus, AssignOp::SubEq },
        { TokenType::ILShift, AssignOp::LShiftEq },
        { TokenType::IRShift, AssignOp::RShiftEq },
        { TokenType::IAnd, AssignOp::AndEq },
        { TokenType::IXor, AssignOp::XorEq },
        { TokenType::IOr, AssignOp::OrEq },
};


const std::map<BinOpExpression::BinOp, std::string> BinOpExpression::OpString = {
        { BinOp::Add, "+" },
        { BinOp::Mul, "*" },
        { BinOp::Div, "/" },
        { BinOp::Mod, "%" },
        { BinOp::Sub, "-" },
        { BinOp::LShift, "<<" },
        { BinOp::RShift, ">>" },
        { BinOp::Leq, "<=" },
        { BinOp::Geq, ">=" },
        { BinOp::Lt, "<" },
        { BinOp::Gt, ">" },
        { BinOp::Eq, "==" },
        { BinOp::Ne, "!=" },
        { BinOp::BinAnd, "&" },
        { BinOp::BinXor, "^" },
        { BinOp::BinOr, "|" },
        { BinOp::LogicAnd, "&&" },
        { BinOp::LogicOr, "||" },
};

const std::map<enum TokenType, BinOpExpression::BinOp> BinOpExpression::TokenMap = {
        { TokenType::Asterisk, BinOp::Mul },
        { TokenType::Div, BinOp::Div },
        { TokenType::Mod, BinOp::Mod },
        { TokenType::Plus, BinOp::Add },
        { TokenType::Minus, BinOp::Sub },
        { TokenType::LShift, BinOp::LShift },
        { TokenType::RShift, BinOp::RShift },
        { TokenType::LessEqual, BinOp::Leq },
        { TokenType::GreaterEqual, BinOp::Geq },
        { TokenType::Less, BinOp::Lt },
        { TokenType::Greater, BinOp::Gt },
        { TokenType::Equal, BinOp::Eq },
        { TokenType::NotEqual, BinOp::Ne },
        { TokenType::Ampersand, BinOp::BinAnd },
        { TokenType::BinOr, BinOp::BinOr },
        { TokenType::BinXor, BinOp::BinXor },
        { TokenType::LogicalAnd, BinOp::LogicAnd },
        { TokenType::LogicalOr, BinOp::LogicOr },
};