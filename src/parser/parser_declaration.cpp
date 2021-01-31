#include "parser.h"

#include "declaration_nodes.h"


NODE(StaticAssertDecl) Parser::ExpectStaticAssert() {
    // _Static_assert( constant-expression , string-literal )
    auto current = Current();
    EatType(TokenType::StaticAssert);
    EatType(TokenType::LParen);
    auto expr = ExpectConstantExpression();
    EatType(TokenType::Comma);
    ExpectType(TokenType::ConstString);
    const std::string& message = std::static_pointer_cast<StringConstant>(Current())->Value;
    EatType(TokenType::ConstString);
    EatType(TokenType::RParen);
    EatType(TokenType::SemiColon);
    return MAKE_NODE(StaticAssertDecl)(current, std::move(expr), message);
}

NODE(TypeSpecifier) Parser::ExpectTypeSpecifier() {
    NODE(TypeSpecifier) type = nullptr;
    auto current = Current();
    if (TypeSpecifier::Is(current->Type)) {
        // straight up keyword
        type = MAKE_NODE(TypeSpecifier)(current, current->Type);
        Advance();
    }
    else if (IsTypedefName(current)) {
        type = MAKE_NODE(TypedefName)(current, std::static_pointer_cast<Identifier>(current)->Name);
        Advance();
    }
    else if (StructSpecifier::Is(current->Type) || UnionSpecifier::Is(current->Type)) {
        return ExpectStructUnionSpecifier();
    }
    else {
        // enum
        log_fatal("Unimplemented type specifier: %s", current->Repr().c_str());
    }
    return type;
}

NODE(InitializerList) Parser::ExpectInitializerList() {
    // initializer list
    auto initializer = MAKE_NODE(InitializerList)(Current());
    EatType(TokenType::LBrace);

    // { (([constexpr] / .identifier)+ =)? initializer (,)? }
    while (true) {
        // list might end in a comma, we checked this at the end of the loop
        auto current = Current();

        // end of initializer list
        if (current->Type == TokenType::RBrace) {
            EatType(TokenType::RBrace);
            return initializer;
        }

        // find all designators (if any)
        std::vector<NODE(Designator)> designators = {};
        for (current = Current(); Is(Current()->Type).AnyOf<TokenType::Dot, TokenType::LBracket>(); current = Current()) {
            Advance();
            if (current->Type == TokenType::Dot) {
                // struct member initializer .identifier
                ExpectType(TokenType::Identifier);
                std::string name = std::static_pointer_cast<Identifier>(Current())->Name;
                designators.push_back(MAKE_NODE(StructFieldDesignator)(current, name));
                Advance();
            }
            else {
                // array field initializer [constant-expression]
                auto expr = ExpectConstantExpression();
                designators.push_back(MAKE_NODE(ArrayMemberDesignator)(current, std::move(expr)));
                EatType(TokenType::RBracket);
            }
        }

        // only an = if there are any designators
        if (!designators.empty()) {
            EatType(TokenType::Assign);
        }

        auto designation_initializer = ExpectInitializer();
        initializer->AddInitializer(std::move(designators), std::move(designation_initializer));

        // either a comma and another initializer-list or end of initializer
        if (Current()->Type != TokenType::RBrace) {
            EatType(TokenType::Comma);
        }
    }
}

NODE(Initializer) Parser::ExpectInitializer() {
    auto current = Current();
    if (current->Type != TokenType::LBrace) {
        // assignment expression initializer
        auto expr = ExpectAssignmentExpression();
        return MAKE_NODE(AssignmentInitializer)(current, std::move(expr));
    }
    return ExpectInitializerList();
}

NODE(Pointer) Parser::ExpectOptPointer() {
    // parse a pointer if it is there
    // * type-qualifier-list(opt)
    if (Current()->Type != TokenType::Asterisk) {
        return nullptr;
    }
    EatType(TokenType::Asterisk);
    auto pointer = MAKE_NODE(Pointer)(Current());
    while (TypeQualifier::Is(Current()->Type)) {
        auto qualifier = MAKE_NODE(TypeQualifier)(Current(), Current()->Type);
        Advance();
        pointer->AddQualifier(qualifier);
    }
    return pointer;
}

NODE(DeclarationSpecifiers) Parser::ExpectDeclarationSpecifiers() {
    // list of storage-class/type-specifier/type-qualifier/function-specifier/alignment-specifier s
    auto specifiers = MAKE_NODE(DeclarationSpecifiers)(Current());
    while (true) {
        auto current = Current();
        if (StorageClassSpecifier::Is(current->Type)) {
            auto specifier = MAKE_NODE(StorageClassSpecifier)(current, current->Type);
            specifiers->AddSpecifier(std::move(specifier));
            Advance();
        }
        else if (TypeQualifier::Is(current->Type)) {
            auto qualifier = MAKE_NODE(TypeQualifier)(current, current->Type);
            specifiers->AddSpecifier(std::move(qualifier));
            Advance();
        }
        else if (FunctionSpecifier::Is(current->Type)) {
            auto specifier = MAKE_NODE(FunctionSpecifier)(current, current->Type);
            specifiers->AddSpecifier(std::move(specifier));
            Advance();
        }
        else if (AlignmentSpecifier::Is(current->Type)) {
            Advance();  // _Alignas
            EatType(TokenType::LParen);
            NODE(AlignmentSpecifier) specifier;
            if (IsTypeName(0)) {
                // _Alignas ( type-name )
                auto type_name = ExpectTypeName();
                specifier = MAKE_NODE(AlignmentSpecifierTypeName)(current, std::move(type_name));
            }
            else {
                // _Alignas ( constant expression )
                auto expr = ExpectConstantExpression();
                specifier = MAKE_NODE(AlignmentSpecifierExpr)(current, std::move(expr));
            }
            specifiers->AddSpecifier(std::move(specifier));
            EatType(TokenType::RParen);
        }
        else if (IsTypeSpecifier(current)) {
            auto specifier = ExpectTypeSpecifier();
            specifiers->AddSpecifier(std::move(specifier));
        }
        else {
            break;
        }
    }

    if (specifiers->Empty()) {
        auto current = Current();
        throw std::runtime_error("Empty declaration specifiers list");
    }

    return specifiers;
}

NODE(DirectDeclaratorParameterListPostfix) Parser::ExpectParameterListPostfix() {
    // START: expect lparen to already be eaten
    // list of parameter declarations, as postfix for (abstract) declarations
    // declaration-specifiers declarator , etc.
    // or
    // declaration-specifiers abstract-declarator(opt) , etc.
    // followed by ... for variadic args
    auto parameter_list = MAKE_NODE(DirectDeclaratorParameterListPostfix)(Current());
    NODE(DeclarationSpecifiers) specifiers;
    NODE(AnyDeclarator) declarator;
    do {
        specifiers = ExpectDeclarationSpecifiers();
        if (!Is(Current()->Type).AnyOf<TokenType::Comma, TokenType::RParen>()) {
            // not end of declarator
            declarator = ExpectDeclarator<AnyDeclarator>();
        }
        else {
            // end of declarator
            declarator = NODE(AbstractDeclarator)(nullptr);
        }
        auto parameter = MAKE_NODE(ParameterDeclaration)(Current(), std::move(specifiers), std::move(declarator));
        parameter_list->AddParameter(std::move(parameter));

        if (Current()->Type == TokenType::Comma) {
            Advance();
            if (Current()->Type == TokenType::Ellipsis) {
                Advance();
                parameter_list->Variadic = true;
                // ellipsis is always at the end
                break;
            }
        }
        else {
            break;
        }
    } while (true);

    return parameter_list;
}


NODE(StructUnionSpecifier) Parser::ExpectStructUnionSpecifier() {
    auto ctx = context("parsing struct / union definition");

    NODE(StructUnionSpecifier) struct_specifier = nullptr;

    // there has to be a next, since struct / union has to be followed by either an identifier or a declaration list
    std::string name;
    if (Next()->Type == TokenType::Identifier) {
        name = std::static_pointer_cast<Identifier>(Next())->Name;
    }

    if (Current()->Type == TokenType::Struct) {
        struct_specifier = MAKE_NODE(StructSpecifier)(Current(), name);
    }
    else if (Current()->Type == TokenType::Union) {
        struct_specifier = MAKE_NODE(UnionSpecifier)(Current(), name);
    }
    else {
        log_fatal("Invalid call to ExpectUnionSpecifier: token not of type 'struct' or 'union': %s", Current()->Repr().c_str());
    }
    Advance();

    bool has_name = false;
    if (Current()->Type == TokenType::Identifier) {
        has_name = true;
        Advance();
    }

    if (!has_name || Current()->Type == TokenType::LBrace) {
        // struct-declaration-list
        Advance();
        while (Current()->Type != TokenType::RBrace) {
            NODE(StructDeclaration) declaration = nullptr;
            auto current = Current();
            if (current->Type == TokenType::StaticAssert) {
                // static assert in struct
                declaration = ExpectStaticAssert();
            }
            else {
                // normal struct declaration
                declaration = MAKE_NODE(StructDeclaration)(Current());
                if (!(TypeQualifier::Is(Current()->Type) || IsTypeSpecifier(Current()))) {
                    throw std::runtime_error("Expected type qualifier or type specifier, got: " + Current()->Repr());
                }

                // specifier-qualifier-list
                while (true) {
                    if (TypeQualifier::Is(Current()->Type)) {
                        auto qualifier = MAKE_NODE(TypeQualifier)(Current(), Current()->Type);
                        Advance();
                        declaration->AddQualifier(std::move(qualifier));
                    }
                    else if (IsTypeSpecifier(Current())) {
                        auto specifier = ExpectTypeSpecifier();
                        declaration->AddSpecifier(std::move(specifier));
                    }
                    else {
                        break;
                    }
                }

                // optional: struct-declarator-list
                if (Current()->Type != TokenType::SemiColon) {
                    do {
                        if (Current()->Type == TokenType::Colon) {
                            // unnamed field (: constant-expression)
                            Advance();
                            auto expr = ExpectConstantExpression();
                            auto field = MAKE_NODE(StructDeclarator)(Current(), std::move(expr));
                            declaration->AddDeclarator(std::move(field));
                        }
                        else {
                            // declarator
                            auto declarator = ExpectDeclarator<Declarator>();

                            if (Current()->Type == TokenType::Colon) {
                                // sized field
                                // declarator: constant-expression
                                Advance();
                                auto expr = ExpectConstantExpression();
                                auto field = MAKE_NODE(StructDeclarator)(Current(), std::move(declarator), std::move(expr));
                                declaration->AddDeclarator(std::move(field));
                            }
                            else {
                                // non-sized field
                                auto field = MAKE_NODE(StructDeclarator)(Current(), std::move(declarator));
                                declaration->AddDeclarator(std::move(field));
                            }
                        }

                        if (Current()->Type == TokenType::Comma) {
                            Advance();
                        }
                        else {
                            break;
                        }
                    } while (true);
                }
                EatType(TokenType::SemiColon);
            }
            struct_specifier->AddDeclaration(declaration);
        }

        EatType(TokenType::RBrace);
    }
    return struct_specifier;
}

NODE(EnumSpecifier) Parser::ExpectEnumSpecifier() {
    EatType(TokenType::Enum);
    NODE(EnumSpecifier) enum_specifier;

    // enum identifier(opt)
    bool has_name = false;
    if (Current()->Type == TokenType::Identifier) {
        enum_specifier = MAKE_NODE(EnumSpecifier)(Current(), std::static_pointer_cast<Identifier>(Current())->Name);
        has_name = true;
        Advance();
    }
    else {
        enum_specifier = MAKE_NODE(EnumSpecifier)(Current());
    }

    // if enum has no name, it must have an explicit enumeration list
    if (!has_name || Current()->Type == TokenType::LBrace) {
        EatType(TokenType::LBrace);
        do {
            ExpectType(TokenType::Identifier);
            NODE(Enumerator) enumerator = nullptr;
            const std::string& name = std::static_pointer_cast<Identifier>(Current())->Name;
            Advance();
            if (Current()->Type == TokenType::Assign) {
                auto current = Current();
                Advance();
                auto expr = ExpectConstantExpression();
                enumerator = MAKE_NODE(Enumerator)(current, name, std::move(expr));
            }
            else {
                enumerator = MAKE_NODE(Enumerator)(Current(), name);
            }
            enum_specifier->AddEnumerator(enumerator);

            if (Current()->Type == TokenType::Comma) {
                Advance();
            }
        } while(Current()->Type != TokenType::RBrace);

        EatType(TokenType::RBrace);
    }
    return enum_specifier;
}

NODE(TypeName) Parser::ExpectTypeName() {
    auto type_name = MAKE_NODE(TypeName)(Current());
    if (!(TypeQualifier::Is(Current()->Type) || IsTypeSpecifier(Current()))) {
        throw std::runtime_error("Expected type qualifier or type specifier, got: " + Current()->Repr());
    }

    // specifier-qualifier-list
    while (true) {
        if (TypeQualifier::Is(Current()->Type)) {
            auto qualifier = MAKE_NODE(TypeQualifier)(Current(), Current()->Type);
            Advance();
            type_name->AddQualifier(std::move(qualifier));
        }
        else if (IsTypeSpecifier(Current())) {
            auto specifier = ExpectTypeSpecifier();
            type_name->AddSpecifier(std::move(specifier));
        }
        else {
            break;
        }
    }

    // abstract-declarator (opt)
    // NOTE: from the grammar: after a type-name is always an RParen
    if (Current()->Type != TokenType::RParen) {
        auto declarator = ExpectDeclarator<Declarator>();
        type_name->Declar = std::move(declarator);
    }

    return type_name;
}

NODE(Declaration) Parser::ExpectDeclaration() {
    auto specifiers = ExpectDeclarationSpecifiers();
    auto declaration = MAKE_NODE(Declaration)(Current(), std::move(specifiers));
    do {
        // init-declarator-list
        auto declarator = ExpectDeclarator<Declarator>();
        auto current = Current();
        if (current->Type == TokenType::Assign) {
            Advance();
            auto initializer = ExpectInitializer();
            auto init_declarator = MAKE_NODE(InitDeclarator)(current, std::move(declarator), std::move(initializer));
            declaration->AddDeclarator(init_declarator);
        }
        else {
            auto init_declarator = MAKE_NODE(InitDeclarator)(current, std::move(declarator));
            declaration->AddDeclarator(init_declarator);
        }

        if (Current()->Type != TokenType::SemiColon) {
            EatType(TokenType::Comma);
        }
        else {
            break;
        }
    } while (true);
    EatType(TokenType::SemiColon);
    return declaration;
}