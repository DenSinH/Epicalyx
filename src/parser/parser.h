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

class StaticAssertDecl;
class Initializer;
class InitializerList;
class AbstractDeclarator;
class Declarator;
class Pointer;
class DeclarationSpecifiers;
class TypeSpecifier;
class ParameterDeclaration;
class DirectDeclaratorParameterListPostfix;
class StructUnionSpecifier;
class EnumSpecifier;
class TypeName;
class Declaration;


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

    NODE(Node) Parse() { return nullptr; };

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
            throw std::runtime_error("Unexpected token, got " + current->Repr() + ", expected: " + Token::TypeString(current->Type));
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

    std::set<std::string> TypedefNames = {};
    bool IsTypedefName(const TOKEN& token);
    bool IsTypeSpecifier(const TOKEN& token);
    bool IsDeclarationSpecifier(const TOKEN& token);
    NODE(TypeSpecifier) ExpectTypeSpecifier();
    bool IsTypeName(size_t after);

    NODE(ExprNode) ExpectPrimaryExpression();
    NODE(ExprNode) ExpectPostfixExpression();
    NODE(ExprNode) ExpectArgumentListExpression();
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
    NODE(AbstractDeclarator) ExpectDeclaratorOrAbstractDeclarator();
    NODE(AbstractDeclarator) ExpectDeclarator();
    NODE(DirectDeclaratorParameterListPostfix) ExpectParameterListPostfix();
    NODE(StructUnionSpecifier) ExpectStructUnionSpecifier();
    NODE(EnumSpecifier) ExpectEnumSpecifier();
    NODE(TypeName) ExpectTypeName();
    NODE(Declaration) ExpectDeclaration();

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

#endif //EPICALYX_PARSER_H
