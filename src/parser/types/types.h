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
        throw std::runtime_error("Type cannot be converted to bool");
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

protected:
    PointerType(CTYPE contained, BaseType base_type) :
            CType(base_type),
            Contained(contained) {

    }
};

struct ArrayType : public PointerType {
    explicit ArrayType(CTYPE contained) : PointerType(contained, BaseType::Array) {

    }

    size_t Size;

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

    bool IsComplete() const override {
        return Fields.size() != 0;
    }

    std::vector<Field> Fields;  // empty if struct was only declared but never defined
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

#endif //EPICALYX_TOKEN_TYPES_H
