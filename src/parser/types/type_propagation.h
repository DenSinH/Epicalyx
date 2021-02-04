#ifndef EPICALYX_TYPE_PROPAGATION_H
#define EPICALYX_TYPE_PROPAGATION_H

#include "types.h"

struct TypePropagation {

    template<typename L, typename R>
    static CTYPE Add(const TYPE(ValueType<L>)& left, const TYPE(ValueType<R>)& right) {
        if (left->has_value() && right->has_value()) {
            return MAKE_TYPE(ValueType<std::common_type_t<L, R>>)(left->value() + right->value(), 0);
        }
        return MAKE_TYPE(ValueType<std::common_type_t<L, R>>)(0);
    }

    template<typename T>
    static CTYPE Add(const TYPE(PointerType)& left, const TYPE(ValueType<T>)& right) {
        if constexpr(!std::is_integral_v<T>) {
            InvalidOperation("+", left, right);
        }
        if (!left->Contained->IsComplete()) {
            IncompleteType(left->Contained);
        }

        return MAKE_TYPE(PointerType)(left->Contained);
    }

    template<typename T>
    static CTYPE Add(const TYPE(ValueType<T>)& left, const TYPE(PointerType)& right) {
        return Add(right, left);
    }

    [[noreturn]] static CTYPE Add(CTYPE& left, CTYPE& right) {
        InvalidOperation("+", left, right);
    }

    template<typename L, typename R>
    static CTYPE Sub(const TYPE(ValueType<L>)& left, const TYPE(ValueType<R>)& right) {
        if (left->has_value() && right->has_value()) {
            return MAKE_TYPE(ValueType<std::common_type_t<L, R>>)(left->value() - right->value(), 0);
        }
        return MAKE_TYPE(ValueType<std::common_type_t<L, R>>)(0);
    }

    template<typename T>
    static CTYPE Sub(const TYPE(PointerType)& left, const TYPE(ValueType<T>)& right) {
        if constexpr(!std::is_integral_v<T>) {
            InvalidOperation("-", left, right);
        }
        if (!left->Contained->IsComplete()) {
            IncompleteType(left->Contained);
        }

        return MAKE_TYPE(PointerType)(left->Contained);
    }

    template<typename T>
    static CTYPE Sub(const TYPE(ValueType<T>)& left, const TYPE(PointerType)& right) {
        // I know this is technically not correct, but we are not propagating a constant here anyway
        return Sub(right, left);
    }

    static CTYPE Sub(const TYPE(PointerType)& left, const TYPE(PointerType)& right) {
        // pointers can be subtracted to find a distance
        if (!left->Contained->IsComplete()) {
            IncompleteType(left->Contained);
        }

        // tood: ptrdiff_t
        return MAKE_TYPE(ValueType<unsigned long long>)(0);
    }

    [[noreturn]] static CTYPE Sub(CTYPE& left, CTYPE& right) {
        InvalidOperation("+", left, right);
    }

private:
    [[noreturn]] static void InvalidOperation(const std::string& op, CTYPE left, CTYPE right) {
        throw std::runtime_error("invalid operands for " + op + ": " + left->to_string() + " and " + right->to_string());
    }

    [[noreturn]] static void IncompleteType(CTYPE right) {
        throw std::runtime_error("operation on incomplete type: " + right->to_string());
    }
};


#endif //EPICALYX_TYPE_PROPAGATION_H
