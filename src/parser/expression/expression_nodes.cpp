#include "primary_expression.h"
#include "parser_state.h"
#include "types/types.h"


bool PrimaryExpressionIdentifier::IsConstant(const ParserState& state) const {
    return state.IsConstant(Name);
}

CTYPE PrimaryExpressionIdentifier::GetType(const ParserState& state) const {
    return state.GetType(Name);
}

template<typename T>
CTYPE PrimaryExpressionConstant<T>::GetType(const ParserState&) const {
    return MAKE_TYPE(ValueType<T>)(Value, CType::LValueNess::None);
}

CTYPE PrimaryStringLiteral::GetType(const ParserState&) const {
    // not assignable (spec says it is undefined)
    TYPE(ValueType<i8>) Char;
    if (Value.empty()) {
        Char = MAKE_TYPE(ValueType<i8>)(0, CType::LValueNess::None);
    }
    else {
        Char = MAKE_TYPE(ValueType<i8>)(Value[0], CType::LValueNess::None);
    }

    // null terminator
    // todo: store string literals
    return MAKE_TYPE(ArrayType)(Char, Value.size() + 1);
}