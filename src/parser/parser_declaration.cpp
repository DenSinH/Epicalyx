#include "parser.h"

#include "declaration_nodes.h"


NODE(StaticAssertDecl) Parser::ExpectStaticAssert() {
    // _Static_assert( constant-expression , string-literal )
    EatType(TokenType::StaticAssert);
    EatType(TokenType::LParen);
    auto expr = ExpectConstantExpression();
    EatType(TokenType::Comma);
    ExpectType(TokenType::ConstString);
    std::string message = std::static_pointer_cast<StringConstant>(Current())->Value;
    EatType(TokenType::ConstString);
    EatType(TokenType::RParen);
    EatType(TokenType::SemiColon);
    return MAKE_NODE(StaticAssertDecl)(expr, message);
}

NODE(TypeSpecifier) Parser::ExpectTypeSpecifier() {
    NODE(TypeSpecifier) type = nullptr;
    auto current = Current();
    if (TypeSpecifier::Is(current->Type)) {
        // straight up keyword
        type = MAKE_NODE(TypeSpecifier)(current->Type);
        Advance();
    }
    else if (IsTypedefName(current)) {
        type = MAKE_NODE(TypedefName)(std::static_pointer_cast<Identifier>(current)->Name);
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
    auto initializer = MAKE_NODE(InitializerList)();
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
                designators.push_back(MAKE_NODE(StructFieldDesignator)(name));
                Advance();
            }
            else {
                // array field initializer [constant-expression]
                auto expr = ExpectConstantExpression();
                designators.push_back(MAKE_NODE(ArrayMemberDesignator)(expr));
                EatType(TokenType::RBracket);
            }
        }

        // only an = if there are any designators
        if (!designators.empty()) {
            EatType(TokenType::Assign);
        }

        auto designation_initializer = ExpectInitializer();
        initializer->AddInitializer(designators, designation_initializer);

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
        return MAKE_NODE(AssignmentInitializer)(expr);
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
    auto pointer = MAKE_NODE(Pointer)();
    while (TypeQualifier::Is(Current()->Type)) {
        auto qualifier = MAKE_NODE(TypeQualifier)(Current()->Type);
        Advance();
        pointer->AddQualifier(qualifier);
    }
    return pointer;
}

NODE(DeclarationSpecifiers) Parser::ExpectDeclarationSpecifiers() {
    // list of storage-class/type-specifier/type-qualifier/function-specifier/alignment-specifier s
    auto specifiers = MAKE_NODE(DeclarationSpecifiers)();
    while (true) {
        auto current = Current();
        if (StorageClassSpecifier::Is(current->Type)) {
            auto specifier = MAKE_NODE(StorageClassSpecifier)(current->Type);
            specifiers->AddSpecifier(specifier);
            Advance();
        }
        else if (TypeQualifier::Is(current->Type)) {
            auto qualifier = MAKE_NODE(TypeQualifier)(current->Type);
            specifiers->AddSpecifier(qualifier);
            Advance();
        }
        else if (FunctionSpecifier::Is(current->Type)) {
            auto specifier = MAKE_NODE(FunctionSpecifier)(current->Type);
            specifiers->AddSpecifier(specifier);
            Advance();
        }
        else if (AlignmentSpecifier::Is(current->Type)) {
            Advance();  // _Alignas
            EatType(TokenType::LParen);
            NODE(AlignmentSpecifier) specifier;
            if (IsTypeName(0)) {
                // _Alignas ( type-name )
                auto type_name = ExpectTypeName();
                specifier = MAKE_NODE(AlignmentSpecifierTypeName)(type_name);
            }
            else {
                // _Alignas ( constant expression )
                auto expr = ExpectConstantExpression();
                specifier = MAKE_NODE(AlignmentSpecifierExpr)(expr);
            }
            specifiers->AddSpecifier(specifier);
            EatType(TokenType::RParen);
        }
        else if (IsTypeSpecifier(current)) {
            auto specifier = ExpectTypeSpecifier();
            specifiers->AddSpecifier(specifier);
        }
        else {
            break;
        }
    }

    if (specifiers->Empty()) {
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
    auto parameter_list = MAKE_NODE(DirectDeclaratorParameterListPostfix)();
    NODE(DeclarationSpecifiers) specifiers;
    NODE(AbstractDeclarator) declarator;
    do {
        specifiers = ExpectDeclarationSpecifiers();
        if (!Is(Current()->Type).AnyOf<TokenType::Comma, TokenType::RParen>()) {
            // not end of declarator
            declarator = ExpectDeclaratorOrAbstractDeclarator();
        }
        else {
            // end of declarator
            declarator = NODE(AbstractDeclarator)(nullptr);
        }
        auto parameter = MAKE_NODE(ParameterDeclaration)(specifiers, declarator);
        parameter_list->AddParameter(parameter);

        if (Current()->Type == TokenType::Comma) {
            Advance();
            if (Current()->Type == TokenType::Ellipsis) {
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

NODE(AbstractDeclarator) Parser::ExpectDeclaratorOrAbstractDeclarator() {
    NODE(AbstractDeclarator) declarator = MAKE_NODE(AbstractDeclarator)();

    // optional pointer
    NODE(Pointer) pointer = ExpectOptPointer();
    while (pointer) {
        declarator->AddPointer(pointer);
        pointer = ExpectOptPointer();
    }

    auto current = Current();
    while (!EndOfStream() && Is(Current()->Type).AnyOf<TokenType::LParen, TokenType::LBracket, TokenType::Identifier>()) {
        current = Current();
        // direct-abstract-declarator
        switch(current->Type) {
            case TokenType::Identifier: {
                // is not abstract declarator, but normal declarator
                // has to be the first thing in the declarator
                if (declarator->IsAbstract()) {
                    if (declarator->Nested) {
                        log_fatal("Nested declarator on abstract declarator -> declarator conversion");
                    }

                    std::string name = std::static_pointer_cast<Identifier>(current)->Name;
                    declarator = MAKE_NODE(Declarator)(name, declarator->Pointers);
                    Advance();
                }
                else {
                    throw std::runtime_error("Got unexpected identifier for non-abstract declarator: " + current->Repr());
                }
                break;
            }
            case TokenType::LParen: {
                // (abstract-declarator) OR (parameter-type-list[opt])
                EatType(TokenType::LParen);
                if (Current()->Type == TokenType::RParen) {
                    // empty parameter-type-list
                    NODE(DirectDeclaratorPostfix) postfix;
                    if (declarator->IsAbstract()) {
                        // type list for abstract declarators
                        postfix = MAKE_NODE(DirectDeclaratorParameterListPostfix)();
                    }
                    else {
                        // otherwise an identifier list
                        postfix = MAKE_NODE(DirectDeclaratorIdentifierListPostfix)();
                    }

                    declarator->AddPostfix(postfix);
                    EatType(TokenType::RParen);
                    break;
                }
                current = Current();
                if (IsDeclarationSpecifier(current)) {
                    // parameter type list
                    NODE(DirectDeclaratorPostfix) parameter_list = ExpectParameterListPostfix();
                    declarator->AddPostfix(parameter_list);
                    EatType(TokenType::RParen);
                    break;
                }
                else if (!declarator->IsAbstract()) {
                    // if the declarator is not abstract, we have already found the identifier or any nested declarators
                    // then we can safely assume there will be an identifier list next
                    // identifier, identifier, etc.
                    NODE(DirectDeclaratorIdentifierListPostfix) ident_list = MAKE_NODE(DirectDeclaratorIdentifierListPostfix)();
                    current = Current();
                    while (current->Type == TokenType::Identifier) {
                        ident_list->AddIdentifier(std::static_pointer_cast<Identifier>(current)->Name);
                        if (Current()->Type == TokenType::Comma) {
                            Advance();
                        }
                        current = Current();
                    }

                    EatType(TokenType::RParen);
                    NODE(DirectDeclaratorPostfix) postfix = std::move(ident_list);
                    declarator->AddPostfix(postfix);
                    break;
                }

                // nested (abstract) declarator
                auto nested_declarator = ExpectDeclaratorOrAbstractDeclarator();
                EatType(TokenType::RParen);
                declarator->SetNested(nested_declarator);
                break;
            }
            case TokenType::LBracket: {
                EatType(TokenType::LBracket);
                current = Current();

                // qualifier-list(opt) * -- for normal declarators
                auto qualifier_list = ExpectListGreedy<TypeQualifier>();

                if (current->Type == TokenType::Asterisk) {
                    EatType(TokenType::RBracket);
                    // [*] postfix
                    NODE(DirectDeclaratorPostfix) array_postfix = MAKE_NODE(DirectDeclaratorArrayPostfix)(false, true);
                    declarator->AddPostfix(array_postfix);
                    break;
                }
                // normal array postfix [qualifiers / static / assignment expression]
                bool Static = Current()->Type == TokenType::Static;
                if (Static) {
                    Advance();
                }
                // static may be before qualifier list
                if (qualifier_list.empty()) {
                    qualifier_list = ExpectListGreedy<TypeQualifier>();
                }

                NODE(ExprNode) expr = nullptr;
                if (Static || (Current()->Type != TokenType::RBracket)) {
                    // if static, has to have an expression, otherwise optional
                    expr = ExpectAssignmentExpression();
                }
                EatType(TokenType::RBracket);

                NODE(DirectDeclaratorPostfix) array_postfix = MAKE_NODE(DirectDeclaratorArrayPostfix)(expr, Static);
                declarator->AddPostfix(array_postfix);
                break;
            }
            default:
                break;
        }
    }

    return declarator;
}

NODE(AbstractDeclarator) Parser::ExpectDeclarator() {
    auto declarator = ExpectDeclaratorOrAbstractDeclarator();
    if (declarator->IsAbstract()) {
        declarator->Print();
        throw std::runtime_error("Abstract declarator not allowed in this context");
    }
    return declarator;
}

NODE(StructUnionSpecifier) Parser::ExpectStructUnionSpecifier() {
    NODE(StructUnionSpecifier) struct_specifier = nullptr;

    if (Current()->Type == TokenType::Struct) {
        struct_specifier = MAKE_NODE(StructSpecifier)();
    }
    else if (Current()->Type == TokenType::Union) {
        struct_specifier = MAKE_NODE(UnionSpecifier)();
    }
    else {
        log_fatal("Invalid call to ExpectUnionSpecifier: token not of type 'struct' or 'union': %s", Current()->Repr().c_str());
    }
    Advance();

    bool has_name = false;
    if (Current()->Type == TokenType::Identifier) {
        // there has to be a next, since struct / union has to be followed by either an identifier or a declaration list
        struct_specifier->ID = std::static_pointer_cast<Identifier>(Current())->Name;
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
                declaration = MAKE_NODE(StructDeclaration)();
                if (!(TypeQualifier::Is(Current()->Type) || IsTypeSpecifier(Current()))) {
                    throw std::runtime_error("Expected type qualifier or type specifier, got: " + Current()->Repr());
                }

                // specifier-qualifier-list
                while (true) {
                    if (TypeQualifier::Is(Current()->Type)) {
                        auto qualifier = MAKE_NODE(TypeQualifier)(Current()->Type);
                        Advance();
                        declaration->AddQualifier(qualifier);
                    }
                    else if (IsTypeSpecifier(Current())) {
                        auto specifier = ExpectTypeSpecifier();
                        declaration->AddSpecifier(specifier);
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
                            auto field = MAKE_NODE(StructDeclarator)(expr);
                            declaration->AddDeclarator(field);
                        }
                        else {
                            // declarator
                            auto declarator = ExpectDeclarator();

                            if (Current()->Type == TokenType::Colon) {
                                // sized field
                                // declarator: constant-expression
                                Advance();
                                auto expr = ExpectConstantExpression();
                                auto field = MAKE_NODE(StructDeclarator)(declarator, expr);
                                declaration->AddDeclarator(field);
                            }
                            else {
                                // non-sized field
                                auto field = MAKE_NODE(StructDeclarator)(declarator);
                                declaration->AddDeclarator(field);
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
    auto enum_specifier = MAKE_NODE(EnumSpecifier)();

    // enum identifier(opt)
    bool has_name = false;
    if (Current()->Type == TokenType::Identifier) {
        enum_specifier->ID = std::static_pointer_cast<Identifier>(Current())->Name;
        has_name = true;
        Advance();
    }

    // if enum has no name, it must have an explicit enumeration list
    if (!has_name || Current()->Type == TokenType::LBrace) {
        EatType(TokenType::LBrace);
        do {
            ExpectType(TokenType::Identifier);
            NODE(Enumerator) enumerator = nullptr;
            std::string name = std::static_pointer_cast<Identifier>(Current())->Name;
            Advance();
            if (Current()->Type == TokenType::Assign) {
                Advance();
                auto expr = ExpectConstantExpression();
                enumerator = MAKE_NODE(Enumerator)(name, expr);
            }
            else {
                enumerator = MAKE_NODE(Enumerator)(name);
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
    auto type_name = MAKE_NODE(TypeName)();
    if (!(TypeQualifier::Is(Current()->Type) || IsTypeSpecifier(Current()))) {
        throw std::runtime_error("Expected type qualifier or type specifier, got: " + Current()->Repr());
    }

    // specifier-qualifier-list
    while (true) {
        if (TypeQualifier::Is(Current()->Type)) {
            auto qualifier = MAKE_NODE(TypeQualifier)(Current()->Type);
            Advance();
            type_name->AddQualifier(qualifier);
        }
        else if (IsTypeSpecifier(Current())) {
            auto specifier = ExpectTypeSpecifier();
            type_name->AddSpecifier(specifier);
        }
        else {
            break;
        }
    }

    // abstract-declarator (opt)
    // NOTE: from the grammar: after a type-name is always an RParen
    if (Current()->Type != TokenType::RParen) {
        auto declarator = ExpectDeclarator();
        type_name->Declar = std::move(declarator);
    }

    return type_name;
}