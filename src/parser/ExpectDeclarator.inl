#include <type_traits>


template<typename T>
NODE(T) Parser::ExpectDeclarator() {
    NODE(T) declarator;

    // optional pointer
    std::vector<NODE(Pointer)> pointers = {};
    NODE(Pointer) pointer = ExpectOptPointer();
    while (pointer) {
        pointers.emplace_back(std::move(pointer));
        pointer = ExpectOptPointer();
    }

    auto current = Current();
    if constexpr(std::is_same_v<T, Declarator>) {
        if (current->Type == TokenType::LParen) {
            Advance();
            auto nested = ExpectDeclarator<Declarator>();
            declarator = MAKE_NODE(Declarator)(current, std::move(nested), std::move(pointers));
            EatType(TokenType::RParen);
        }
        else {
            EatType(TokenType::Identifier);
            const std::string& name = std::static_pointer_cast<Identifier>(current)->Name;
            declarator = MAKE_NODE(Declarator)(current, name, std::move(pointers));
        }
    }
    else {
        declarator = MAKE_NODE(AbstractDeclarator)(current, std::move(pointers));
    }

    while (!EndOfStream() && Is(Current()->Type).AnyOf<TokenType::LParen, TokenType::LBracket, TokenType::Identifier>()) {
        current = Current();
        // direct-abstract-declarator
        switch(current->Type) {
            case TokenType::Identifier: {
                // is not abstract declarator, but normal declarator
                // has to be the first thing in the declarator
                if constexpr (!std::is_same_v<T, Declarator>) {
                    if (declarator->HasNested()) {
                        log_fatal("Nested declarator on abstract declarator -> declarator conversion");
                    }
                    const std::string& name = std::static_pointer_cast<Identifier>(current)->Name;
                    declarator = MAKE_NODE(Declarator)(current, name, std::move(declarator->Pointers));
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
                        postfix = MAKE_NODE(DirectDeclaratorParameterListPostfix)(Current());
                    }
                    else {
                        // otherwise an identifier list
                        postfix = MAKE_NODE(DirectDeclaratorIdentifierListPostfix)(Current());
                    }

                    declarator->AddPostfix(std::move(postfix));
                    EatType(TokenType::RParen);
                    break;
                }
                current = Current();
                if (IsDeclarationSpecifier(current)) {
                    // parameter type list
                    NODE(DirectDeclaratorPostfix) parameter_list = ExpectParameterListPostfix();
                    declarator->AddPostfix(std::move(parameter_list));
                    EatType(TokenType::RParen);
                    break;
                }
                else if (!declarator->IsAbstract()) {
                    // if the declarator is not abstract, we have already found the identifier or any nested declarators
                    // then we can safely assume there will be an identifier list next
                    // identifier, identifier, etc.
                    current = Current();
                    NODE(DirectDeclaratorIdentifierListPostfix) ident_list = MAKE_NODE(DirectDeclaratorIdentifierListPostfix)(current);
                    while (current->Type == TokenType::Identifier) {
                        ident_list->AddIdentifier(std::static_pointer_cast<Identifier>(current)->Name);
                        if (Current()->Type == TokenType::Comma) {
                            Advance();
                        }
                        current = Current();
                    }

                    EatType(TokenType::RParen);
                    NODE(DirectDeclaratorPostfix) postfix = std::move(ident_list);
                    declarator->AddPostfix(std::move(postfix));
                    break;
                }

                // nested (abstract) declarator
                if constexpr(std::is_same_v<T, Declarator>) {
                    log_fatal("Nested operator for non-abstract declarator must be detected already");
                }
                else {
                    auto nested_declarator = ExpectDeclarator<T>();
                    EatType(TokenType::RParen);
                    declarator->Declarator::SetNested(std::move(nested_declarator));
                }
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
                    NODE(DirectDeclaratorPostfix) array_postfix = MAKE_NODE(DirectDeclaratorArrayPostfix)(current, false, true);
                    declarator->AddPostfix(std::move(array_postfix));
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

                NODE(DirectDeclaratorPostfix) array_postfix = MAKE_NODE(DirectDeclaratorArrayPostfix)(current, std::move(expr), Static);
                declarator->AddPostfix(std::move(array_postfix));
                break;
            }
            default:
                break;
        }
    }

    return declarator;
}