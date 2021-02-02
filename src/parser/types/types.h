#ifndef EPICALYX_TYPES_H
#define EPICALYX_TYPES_H

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <stdexcept>

#include "log.h"


#define TYPE std::shared_ptr<CType>

struct CType {
    using NumericValue = std::variant<long double, unsigned long long>;
    using OptionalNumericValue = std::optional<NumericValue>;

    enum class BaseType {
        Void,
        Char,
        Short,
        Int,
        Long,
        LongLong,
        Float,
        Double,
        Bool,
        Complex,

        Function,
        Pointer,
        Array,
        // todo: Atomic,
        Struct,
        Union,
        Enum
        // typedef is just another type
    };

    enum TypeFlags : unsigned {
        SpecifierUnsigned  = 0x01,

        StorageExtern      = 0x02,
        StorageStatic      = 0x04,
        StorageThreadLocal = 0x08,
        StorageRegister    = 0x10,

        QualifierConst     = 0x20,
        QualifierRestrict  = 0x40,
        QualifierVolatile  = 0x80,
        QualifierAtomic    = 0x100,
    };

    explicit CType(BaseType type) : Base(type) {

    }

    virtual OptionalNumericValue GetOptionalValue() const noexcept {
        return {};
    }

    virtual NumericValue GetNumericValue() const {
        throw std::runtime_error("Expected constant value");
    }

    long double GetFloatValue() const {
        return std::get<long double>(GetNumericValue());
    }

    unsigned long long GetIntValue() const {
        return std::get<unsigned long long>(GetNumericValue());
    }

    bool GetBoolValue() const {
        auto value = GetNumericValue();
        if (std::holds_alternative<long double>(value)) {
            return GetFloatValue() != 0;
        }
        else if (std::holds_alternative<unsigned long long>(value)) {
            return GetIntValue() != 0;
        }
        log_fatal("Unimplemented value type");
    }

    const BaseType Base;
    unsigned Flags = 0;
};

struct ValueType : public CType {
    explicit ValueType(BaseType base) :
            CType(base),
            Value() {

    }

    explicit ValueType(BaseType base, long double value) :
            CType(base),
            Value(value) {

    }

    explicit ValueType(BaseType base, unsigned long long value) :
            CType(base),
            Value(value) {

    }

    OptionalNumericValue GetOptionalValue() const noexcept override {
        return Value;
    }

    NumericValue GetNumericValue() const override {
        if (Value.has_value()) {
            return Value.value();
        }
        throw std::runtime_error("Numerical value is not a constant");
    }

    const OptionalNumericValue Value;
};

struct PointerType : public CType {
    explicit PointerType() : CType(BaseType::Pointer) {

    }

    std::shared_ptr<CType> Contained;
};

struct ArrayType : public CType {
    explicit ArrayType() : CType(BaseType::Array) {

    }

    size_t Size;
    std::shared_ptr<CType> Contained;
};

struct FunctionType : public CType {
    explicit FunctionType() : CType(BaseType::Function) {

    }

    std::shared_ptr<CType> ReturnType;
    std::vector<std::shared_ptr<CType>> ArgTypes;
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
};

struct UnionType : public StructUnionType<CType::BaseType::Union> {
    explicit UnionType() : StructUnionType() {

    }
};

#endif //EPICALYX_TOKEN_TYPES_H
