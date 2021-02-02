#include "primary_expression.h"
#include "parser_state.h"


bool PrimaryExpressionIdentifier::IsConstant(const ParserState& state) const {
    return state.IsConstant(Name);
}