#ifndef EPICALYX_TYPES_H
#define EPICALYX_TYPES_H

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <stdexcept>
#include <type_traits>

#include "type_utils.h"
#include "log.h"

#define TYPE(...) std::shared_ptr<__VA_ARGS__>
#define MAKE_TYPE(...) std::make_shared<__VA_ARGS__>
#define CTYPE TYPE(CType)

#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

struct CType {
    enum class BaseType {
        Void,
        Value,
        Function,
        Pointer,
        Array,
        // todo: Atomic,
        Struct,
        Union,
        // enum is just an int
        // typedef is just another type
    };

    enum TypeFlags : unsigned {
        StorageExtern      = 0x01,
        StorageStatic      = 0x02,
        StorageThreadLocal = 0x04,
        StorageRegister    = 0x08,

        QualifierConst     = 0x10,
        QualifierRestrict  = 0x20,
        QualifierVolatile  = 0x40,
        QualifierAtomic    = 0x80,
    };

    explicit CType(BaseType type, unsigned flags = 0) : Base(type), Flags(flags) {

    }

    virtual bool IsComplete() const {
        return false;
    }

    virtual bool GetBoolValue() const {
        return false;
    }

    virtual std::string to_string() const noexcept {
        log_fatal("Unimplemented ctype");
    };

    const BaseType Base;
    unsigned Flags = 0;
};

template<typename T>
struct ValueType : public CType {
    explicit ValueType(unsigned flags) :
            CType(BaseType::Value, flags),
            Value() {

    }

    explicit ValueType(T value, unsigned flags) :
            CType(BaseType::Value),
            Value(value) {

    }

    bool GetBoolValue() const override {
        if (Value.has_value()) {
            return Value.value() != 0;
        }
        throw std::runtime_error("Bool value requested from non-constant value");
    }

    bool IsComplete() const override {
        return true;
    }

    constexpr T value() const {
        return Value.value();
    }

    constexpr bool has_value() const {
        return Value.has_value();
    }

    std::string to_string() const noexcept override {
        if (!Value.has_value()) {
            return type_string_v<T>;
        }
        return type_string_v<T> + ": " + std::to_string(Value.value());
    }

    std::optional<T> Value;
};

struct PointerType : public CType {
    explicit PointerType(CTYPE contained) :
            CType(BaseType::Pointer),
            Contained(contained) {

    }

    const CTYPE Contained;

    bool IsComplete() const override {
        return true;
    }

    std::string to_string() const noexcept override {
        return "(" + Contained->to_string() + ") *";
    }
};

struct ArrayType : public CType {
    explicit ArrayType() : CType(BaseType::Array) {

    }

    size_t Size;
    CTYPE Contained;

    bool IsComplete() const override {
        return true;
    }

    std::string to_string() const noexcept override {
        return "(" + Contained->to_string() + ")[" + std::to_string(Size) + "]";
    }
};

struct FunctionType : public CType {
    explicit FunctionType() : CType(BaseType::Function) {

    }

    CTYPE ReturnType;
    std::vector<CTYPE> ArgTypes;

    bool IsComplete() const override {
        return true;
    }

    std::string to_string() const noexcept override {
        std::string repr = ReturnType->to_string() + "(";
        for (auto& a : ArgTypes) {
            repr += a->to_string() + ",";
        }
        return repr + ")";
    }
};

template<CType::BaseType type>
struct StructUnionType : public CType {
    explicit StructUnionType() : CType(type) {

    }

    struct Field {
        Field(std::string name, size_t size, std::shared_ptr<CType> contained) :
            Name(std::move(name)),
            Size(size),
            Contained(std::move(contained)) {

        }

        Field(std::string name, std::shared_ptr<CType> contained) :
            Name(std::move(name)),
            Size(0),
            Contained(std::move(contained)) {

        }

        const std::string Name;
        const size_t Size = 0;  // 0 means default size
        std::shared_ptr<CType> Contained;
    };

    void AddField(const std::string& name, size_t size, std::shared_ptr<CType> contained) {
        Fields.push_back(Field(name, size, contained));
    }

    void AddField(const std::string& name, std::shared_ptr<CType> contained) {
        Fields.push_back(Field(name, contained));
    }

    std::vector<Field> Fields;
};

struct StructType : public StructUnionType<CType::BaseType::Struct> {
    explicit StructType() : StructUnionType() {

    }

    std::string to_string() const noexcept override {
        return "struct";
    }
};

struct UnionType : public StructUnionType<CType::BaseType::Union> {
    explicit UnionType() : StructUnionType() {

    }

    std::string to_string() const noexcept override {
        return "union";
    }
};

struct TypePropagation {

    template<typename L, typename R>
    static CTYPE Add(const TYPE(ValueType<L>) left, const TYPE(ValueType<R>) right) {
        if (left->has_value() && right->has_value()) {
            return MAKE_TYPE(ValueType<std::common_type_t<L, R>>)(left->value() + right->value(), 0);
        }
        return MAKE_TYPE(ValueType<std::common_type_t<L, R>>)(0);
    }

    template<typename T>
    static CTYPE Add(const TYPE(PointerType) left, const TYPE(ValueType<T>) right) {
        if constexpr(!std::is_integral_v<T>) {
            InvalidOperation("+", left, right);
        }
        if (!left->Contained->IsComplete()) {
            IncompleteType(left->Contained);
        }

        return MAKE_TYPE(PointerType)(left->Contained);
    }

    template<typename T>
    static CTYPE Add(const TYPE(ValueType<T>) left, const TYPE(PointerType) right) {
        return Add(right, left);
    }

    template<typename T>
    static CTYPE Add(const TYPE(ArrayType) left, const TYPE(ValueType<T>) right) {
        if constexpr(!std::is_integral_v<T>) {
            InvalidOperation("+", left, right);
        }

        return MAKE_TYPE(PointerType)(left->Contained);
    }

    template<typename T>
    static CTYPE Add(const TYPE(ValueType<T>) left, const TYPE(ArrayType) right) {
        return Add(right, left);
    }

    static CTYPE Add(CTYPE left, CTYPE right) {
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

#endif //EPICALYX_TOKEN_TYPES_H
