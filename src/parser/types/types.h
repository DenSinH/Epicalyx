#ifndef EPICALYX_TYPES_H
#define EPICALYX_TYPES_H

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <stdexcept>
#include <type_traits>

#include "type_utils.h"
#include "default.h"
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

template<typename T> struct ValueType;
struct VoidType;
struct PointerType;
struct ArrayType;
struct FunctionType;
struct StructType;
struct UnionType;

#define INTEGRAL_CTYPE_SIGNATURES(MACRO, ...) \
    MACRO(__VA_ARGS__, ValueType<u8>) \
    MACRO(__VA_ARGS__, ValueType<i8>) \
    MACRO(__VA_ARGS__, ValueType<i16>) \
    MACRO(__VA_ARGS__, ValueType<u16>) \
    MACRO(__VA_ARGS__, ValueType<i32>) \
    MACRO(__VA_ARGS__, ValueType<u32>) \
    MACRO(__VA_ARGS__, ValueType<i64>) \
    MACRO(__VA_ARGS__, ValueType<u64>) \

#define NUMERIC_CTYPE_SIGNATURES(MACRO, ...) \
    INTEGRAL_CTYPE_SIGNATURES(MACRO, __VA_ARGS__) \
    MACRO(__VA_ARGS__, ValueType<float>) \
    MACRO(__VA_ARGS__, ValueType<double>)    \

#define CTYPE_SIGNATURES(MACRO, ...) \
    NUMERIC_CTYPE_SIGNATURES(MACRO, __VA_ARGS__) \
    MACRO(__VA_ARGS__, VoidType) \
    MACRO(__VA_ARGS__, PointerType) \
    MACRO(__VA_ARGS__, ArrayType) \
    MACRO(__VA_ARGS__, FunctionType) \
    MACRO(__VA_ARGS__, StructType) \
    MACRO(__VA_ARGS__, UnionType)

#define VIRTUAL_BINOP_HANDLERS(handler) CTYPE_SIGNATURES(VIRTUAL_BINOP_HANDLER, handler)

#define BINOP_HANDLER(handler, type) CTYPE handler(const type& other) const
#define UNOP_HANDLER(handler) CTYPE handler() const
#define VIRTUAL_CASTABLE(_, type) virtual bool CastableTypeImpl(const type& other) const { return false; }
#define VIRTUAL_EQ(_, type) virtual bool EqualTypeImpl(const type& other) const { return false; }
#define VIRTUAL_BINOP_HANDLER(handler, type) virtual BINOP_HANDLER(handler, type) { throw std::runtime_error("Invalid types for operand"); }
#define VIRTUAL_BINOP(handler) public: virtual CTYPE handler(const CType& other) const { return nullptr; }; protected: VIRTUAL_BINOP_HANDLERS(R ## handler)
#define VIRTUAL_UNOP(handler) virtual UNOP_HANDLER(handler) { return nullptr; }

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

    virtual std::string ToString() const noexcept {
        log_fatal("Unimplemented ctype");
    };

    template<typename T>
    friend struct ValueType;
    friend struct VoidType;
    friend struct PointerType;
    friend struct ArrayType;
    friend struct FunctionType;
    template<CType::BaseType type>
    friend struct StructUnionType;
    friend struct StructType;
    friend struct UnionType;

    VIRTUAL_BINOP(Add)
    VIRTUAL_BINOP(Sub)

    VIRTUAL_BINOP(Mul)
    VIRTUAL_BINOP(Div)
    VIRTUAL_BINOP(Mod)

    VIRTUAL_BINOP(Xor)
    VIRTUAL_BINOP(BinAnd)
    VIRTUAL_BINOP(BinOr)

    TYPE(ValueType<i32>) LogAnd(const CTYPE& other) const;
    TYPE(ValueType<i32>) LogOr(const CTYPE& other) const;

    VIRTUAL_BINOP(Lt)
    VIRTUAL_BINOP(Gt)
    VIRTUAL_BINOP(Eq)

    VIRTUAL_BINOP(LShift)
    VIRTUAL_BINOP(RShift)

    VIRTUAL_UNOP(Deref)
    VIRTUAL_UNOP(Ref)
    VIRTUAL_UNOP(Neg)
    VIRTUAL_UNOP(BinNot)
    VIRTUAL_UNOP(Not)

public:
    // for checking actual equality:
    virtual bool CastableType(const CType& other) const {
        return false;
    }

    virtual bool EqualType(const CType& other) const {
        return false;
    }

    virtual TYPE(ValueType<i32>) BoolVal() const {
        throw std::runtime_error("Type cannot be cast to bool");
    }

    const BaseType Base;
    unsigned Flags = 0;

protected:
    CTYPE_SIGNATURES(VIRTUAL_CASTABLE)  // compatible types (can cast)
    CTYPE_SIGNATURES(VIRTUAL_EQ)        // equal types
};


#define OVERRIDE_BINOP_HANDLER(handler, type) BINOP_HANDLER(handler, type) override
#define OVERRIDE_BINOP_HANDLER_NOIMPL(handler, type) OVERRIDE_BINOP_HANDLER(handler, type);
#define OVERRIDE_BINOP(handler) BINOP_HANDLER(handler, CType) override { return other.R ## handler(*this); }
#define OVERRIDE_UNOP(_operator) CTYPE _operator() const override;
#define OVERRIDE_BASE_CASTABLE bool CastableType(const CType& other) const override { return other.CastableType(*this); }
#define OVERRIDE_BASE_EQ bool EqualType(const CType& other) const override { return other.EqualTypeImpl(*this); }

struct VoidType : public CType {
    explicit VoidType(unsigned flags) :
        CType(BaseType::Void, flags) {
        
    }
    
    bool IsComplete() const override {
        return false;
    }

    bool CastableType(const CType& other) const override {
        // any type can be cast to void
        return true;
    }

    OVERRIDE_BASE_EQ

    std::string ToString() const noexcept override {
        return "void";
    };

protected:
    bool EqualTypeImpl(const VoidType& other) const override {
        return true;
    }
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
        throw std::runtime_error("Bool value requested from non-constant Get");
    }

    bool IsComplete() const override {
        return true;
    }

    constexpr T Get() const {
        return Value.value();
    }

    constexpr bool HasValue() const {
        return Value.has_value();
    }

    TYPE(ValueType<i32>) BoolVal() const override {
        if (HasValue()) {
            return MAKE_TYPE(ValueType<i32>)(Value.value() != 0, 0);
        }
        return MAKE_TYPE(ValueType<i32>)(0);
    }

    std::string ToString() const noexcept override {
        if (!Value.has_value()) {
            return type_string_v<T>;
        }
        return type_string_v<T> + ": " + std::to_string(Value.value());
    }

    OVERRIDE_BASE_CASTABLE
    OVERRIDE_BASE_EQ

    OVERRIDE_BINOP(Add)
    OVERRIDE_BINOP(Sub)

    OVERRIDE_BINOP(Mul);
    OVERRIDE_BINOP(Div);
    OVERRIDE_BINOP(Mod);

    OVERRIDE_BINOP(Xor);
    OVERRIDE_BINOP(BinAnd);
    OVERRIDE_BINOP(BinOr);

    OVERRIDE_BINOP(LShift);
    OVERRIDE_BINOP(RShift);

    OVERRIDE_BINOP(Lt);
    OVERRIDE_BINOP(Gt);
    OVERRIDE_BINOP(Eq);

    std::optional<T> Value;

protected:
    // perform BinOp on other in reverse: so this.ValueTypeRBinOp<std::plus> <==> other + this
    template<typename L>
    CTYPE ValueTypeRBinOp(
            const ValueType<L>& other,
            std::function<std::common_type_t<L, T>(std::common_type_t<L, T>, std::common_type_t<L, T>)> handler
    ) const {
        if (HasValue() && other.HasValue()) {
            return MAKE_TYPE(ValueType<std::common_type_t<L, T>>)(handler(other.Get(), Get()), 0);
        }
        return MAKE_TYPE(ValueType<std::common_type_t<L, T>>)(0);
    }

    template<typename L>
    CTYPE ValueTypeRBoolBinOp(
            const ValueType<L>& other,
            std::function<i8(std::common_type_t<L, T>, std::common_type_t<L, T>)> handler
    ) const {
        if (HasValue() && other.HasValue()) {
            return MAKE_TYPE(ValueType<i8>)(handler(other.Get(), Get()), 0);
        }
        return MAKE_TYPE(ValueType<i8>)(0);
    }

    NUMERIC_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RAdd)
    OVERRIDE_BINOP_HANDLER(RAdd, PointerType);
    NUMERIC_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RSub)
    OVERRIDE_BINOP_HANDLER(RSub, PointerType);

    NUMERIC_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RMul)
    NUMERIC_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RDiv)
    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RMod)

    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLShift)
    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RRShift)

    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RXor)
    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RBinAnd)
    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RBinOr)

    NUMERIC_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLt)
    OVERRIDE_BINOP_HANDLER(RLt, PointerType);
    NUMERIC_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RGt)
    OVERRIDE_BINOP_HANDLER(RGt, PointerType);
    NUMERIC_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, REq)
    OVERRIDE_BINOP_HANDLER(REq, PointerType);

    bool CastableTypeImpl(const ValueType<u8>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<i8>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<u16>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<i16>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<u32>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<i32>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<u64>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<i64>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<float>& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<double>& other) const override { return true; }

    bool EqualTypeImpl(const ValueType<T>& other) const override {
        return true;
    }
};

struct PointerType : public CType {
    explicit PointerType(CTYPE contained, unsigned flags = 0) :
            CType(BaseType::Pointer, flags),
            Contained(std::move(contained)) {

    }

    const CTYPE Contained;

    bool IsComplete() const override {
        return true;
    }

    std::string ToString() const noexcept override {
        return "(" + Contained->ToString() + ") *";
    }

    TYPE(ValueType<i32>) BoolVal() const override {
        return MAKE_TYPE(ValueType<i32>)(0);
    }

    OVERRIDE_BASE_CASTABLE
    OVERRIDE_BASE_EQ

    OVERRIDE_BINOP(Add)
    OVERRIDE_BINOP(Sub)

    OVERRIDE_BINOP(Lt);
    OVERRIDE_BINOP(Gt);
    OVERRIDE_BINOP(Eq);

protected:
    PointerType(CTYPE contained, BaseType base_type) :
            CType(base_type),
            Contained(std::move(contained)) {

    }

    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RAdd)
    // we cannot do `int - ptr_type`, so we must not override this
    OVERRIDE_BINOP_HANDLER(RSub, PointerType);

    OVERRIDE_BINOP_HANDLER(RLt, PointerType);
    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLt)
    OVERRIDE_BINOP_HANDLER(RGt, PointerType);
    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RGt)
    OVERRIDE_BINOP_HANDLER(REq, PointerType);
    INTEGRAL_CTYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, REq)

    bool CastableTypeImpl(const PointerType&) const override { return true; }
    bool CastableTypeImpl(const ValueType<i8>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u8>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<i16>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u16>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<i32>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u32>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<i64>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u64>&) const override { return true; }

    bool EqualTypeImpl(const PointerType& other) const override {
        return (*Contained).EqualType(*other.Contained);
    }
};

struct ArrayType : public PointerType {
    explicit ArrayType(CTYPE contained) : PointerType(std::move(contained), BaseType::Array) {

    }

    size_t Size;

    bool IsComplete() const override {
        return true;
    }

    std::string ToString() const noexcept override {
        return "(" + Contained->ToString() + ")[" + std::to_string(Size) + "]";
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

    TYPE(ValueType<i32>) BoolVal() const override {
        return MAKE_TYPE(ValueType<i32>)(1, 0);  // always true
    }

    std::string ToString() const noexcept override {
        std::string repr = ReturnType->ToString() + "(";
        for (auto& a : ArgTypes) {
            repr += a->ToString() + ",";
        }
        return repr + ")";
    }

    OVERRIDE_BASE_CASTABLE
    OVERRIDE_BASE_EQ

protected:
    bool CastableTypeImpl(const FunctionType& other) const override { return true; }
    bool CastableTypeImpl(const PointerType& other) const override { return true; }
    bool CastableTypeImpl(const ValueType<i8>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u8>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<i16>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u16>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<i32>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u32>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<i64>&) const override { return true; }
    bool CastableTypeImpl(const ValueType<u64>&) const override { return true; }

    bool EqualTypeImpl(const FunctionType& other) const override {
        if (!(*ReturnType).EqualType(*other.ReturnType)) {
            return false;
        }
        if (ArgTypes.size() != other.ArgTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < ArgTypes.size(); i++) {
            if (!(*ArgTypes[i]).EqualType(*other.ArgTypes[i])) {
                return false;
            }
        }

        return true;
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
            Type(std::move(contained)) {

        }

        Field(std::string name, std::shared_ptr<CType> contained) :
            Name(std::move(name)),
            Size(0),
            Type(std::move(contained)) {

        }

        const std::string Name;
        const size_t Size = 0;  // 0 means default size
        std::shared_ptr<CType> Type;
    };

    void AddField(const std::string& name, size_t size, std::shared_ptr<CType> contained) {
        Fields.push_back(Field(name, size, contained));
    }

    void AddField(const std::string& name, std::shared_ptr<CType> contained) {
        Fields.push_back(Field(name, contained));
    }

    bool IsComplete() const override {
        return !Fields.empty();
    }

    std::vector<Field> Fields;  // empty if struct was only declared but never defined

protected:
    bool _CastableTypeImpl(const StructUnionType<type>& other) const {
        return _EqualTypeImpl(other);
    }

    bool _EqualTypeImpl(const StructUnionType<type>& other) const {
        if (Fields.size() != other.Fields.size()) {
            return false;
        }

        if (!IsComplete()) {
            return false;
        }

        for (size_t i = 0; i < Fields.size(); i++) {
            const auto& this_field = Fields[i];
            const auto& other_field = other.Fields[i];
            if (this_field.Name != other_field.Name) {
                return false;
            }
            if (this_field.Size != other_field.Size) {
                return false;
            }
            if (!(*this_field.Type).EqualType(*other_field.Type)) {
                return false;
            }
        }

        return true;
    }
};

struct StructType : public StructUnionType<CType::BaseType::Struct> {
    explicit StructType() : StructUnionType() {

    }

    std::string ToString() const noexcept override {
        return "struct";
    }

    OVERRIDE_BASE_CASTABLE
    OVERRIDE_BASE_EQ

private:
    bool CastableTypeImpl(const StructType& other) const override {
        return _CastableTypeImpl(other);
    }

    bool EqualTypeImpl(const StructType& other) const override {
        return _EqualTypeImpl(other);
    }
};

struct UnionType : public StructUnionType<CType::BaseType::Union> {
    explicit UnionType() : StructUnionType() {

    }

    std::string ToString() const noexcept override {
        return "union";
    }

    OVERRIDE_BASE_CASTABLE
    OVERRIDE_BASE_EQ

private:
    bool CastableTypeImpl(const UnionType& other) const override {
        return _CastableTypeImpl(other);
    }

    bool EqualTypeImpl(const UnionType& other) const override {
        return _EqualTypeImpl(other);
    }
};

#endif //EPICALYX_TOKEN_TYPES_H
