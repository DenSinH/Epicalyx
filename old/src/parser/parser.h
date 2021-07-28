#ifndef EPICALYX_PARSER_H
#define EPICALYX_PARSER_H

#include "../tokenizer/tokenizer.h"
#include "../tokenizer/tokens.h"
#include "../state/state.h"
#include "AST.h"
#include "log.h"

#include <set>
#include <stdexcept>
#include <variant>

#include "declaration/declaration_nodes.h"
#include "statement/statement_nodes.h"


class Parser : public Stateful {
public:
    explicit Parser(std::vector<TOKEN>& tokens) {
        this->Tokens = tokens;
    }

    explicit Parser(Tokenizer& tokenizer) {
        this->Tokens = tokenizer.Tokens;
    }

    explicit Parser(Tokenizer* tokenizer) {
        this->Tokens = tokenizer->Tokens;
    }

    std::vector<NODE(Node)> Parse();

    TOKEN Current() {
        if (Index < Tokens.size()) {
            return Tokens[Index];
        }
        throw std::runtime_error("Unexpected end of program");
    }

    void Advance() {
        Index++;
    }

    bool EndOfStream() {
        return Index >= Tokens.size();
    }

    bool HasAfter(size_t offset) {
        return (Index + offset) < (Tokens.size() - 1);
    }

    bool HasNext() {
        return Index < (Tokens.size() - 1);
    }

    TOKEN Next() {
        return Tokens[Index + 1];
    }

    TOKEN LookAhead(int amount) {
        if (Index + amount < Tokens.size()) {
            return Tokens[Index + amount];
        }
        return nullptr;
    }

    TOKEN ExpectType(enum TokenType type) {
        auto current = Current();
        if (current->Type != type) {
            throw std::runtime_error("Unexpected token, got " + current->Repr() + ", expected: " + Token::TypeString(type));
        }
        return current;
    }

    TOKEN EatType(enum TokenType type) {
        auto eaten = ExpectType(type);
        Advance();
        return eaten;
    }
    
private:
    friend int main();
    std::vector<TOKEN> Tokens;
    unsigned long long Index = 0;

    std::vector<std::set<std::string>> TypedefNames = {{}};  // global scope initially
    std::set<std::string> _TypedefNames = {};
    bool IsTypedefName(const TOKEN& token);
    bool IsTypeSpecifier(const TOKEN& token);
    bool IsDeclarationSpecifier(const TOKEN& token);
    NODE(TypeSpecifier) ExpectTypeSpecifier();
    bool IsTypeName(size_t after);

    NODE(ExprNode) ExpectPrimaryExpression();
    NODE(ExprNode) ExpectPostfixExpression();
    NODE(ExprNode) ExpectUnaryExpression();
    NODE(ExprNode) ExpectCastExpressionOrTypeInitializer();
    NODE(ExprNode) ExpectMultExpression();
    NODE(ExprNode) ExpectAddExpression();
    NODE(ExprNode) ExpectShiftExpression();
    NODE(ExprNode) ExpectRelationalExpression();
    NODE(ExprNode) ExpectEqualityExpression();
    NODE(ExprNode) ExpectBinAndExpression();
    NODE(ExprNode) ExpectBinXorExpression();
    NODE(ExprNode) ExpectBinOrExpression();
    NODE(ExprNode) ExpectLogicAndExpression();
    NODE(ExprNode) ExpectLogicOrExpression();
    NODE(ExprNode) ExpectConditionalExpression();
    NODE(ExprNode) ExpectConstantExpression() { return ExpectConditionalExpression(); }
    NODE(ExprNode) ExpectAssignmentExpression();
    NODE(ExprNode) ExpectExpression();

    NODE(StaticAssertDecl) ExpectStaticAssert();
    NODE(Initializer) ExpectInitializer();
    NODE(InitializerList) ExpectInitializerList();
    NODE(Pointer) ExpectOptPointer();
    NODE(DeclarationSpecifiers) ExpectDeclarationSpecifiers();
    template<typename T>
    NODE(T) ExpectDeclarator();
    NODE(AbstractDeclarator) ExpectDeclarator();
    NODE(DirectDeclaratorParameterListPostfix) ExpectParameterListPostfix();
    NODE(StructUnionSpecifier) ExpectStructUnionSpecifier();
    NODE(EnumSpecifier) ExpectEnumSpecifier();
    NODE(TypeName) ExpectTypeName();
    NODE(Declaration) ExpectDeclaration();

    NODE(StatementNode) ExpectStatement();
    NODE(CompoundStatement) ExpectCompoundStatement();

    NODE(Node) ExpectTranslationUnit();

    template<typename T>
    std::vector<NODE(T)> ExpectListGreedy() {
        std::vector<NODE(T)> list = {};
        auto current = Current();
        while (T::Is(current->Type)) {
            list.push_back(MAKE_NODE(T)(current, current->Type));
            Advance();
            current = Current();
        }
        return list;
    }

    template<
            NODE(ExprNode) (Parser::*SubNode)(),
            enum TokenType... type
    > NODE(ExprNode) ExpectBinOpExpression();
};

#include "ExpectBinOp.inl"
#include "ExpectDeclarator.inl"

#endif //EPICALYX_PARSER_H