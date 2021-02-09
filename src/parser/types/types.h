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
#define VIRTUAL_BINOP(handler) public: virtual CTYPE handler(const CType& other) const { throw std::runtime_error("Unimplemented binop handler"); }; protected: VIRTUAL_BINOP_HANDLERS(R ## handler)
#define VIRTUAL_UNOP(handler) public: virtual UNOP_HANDLER(handler) { throw std::runtime_error("Unimplemented unop handler"); }

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

    enum class LValueNess {
        None = 0,
        LValue,
        Assignable,
    };

    explicit CType(
            BaseType type,
            LValueNess lvalue,
            unsigned flags = 0
        ) :
                Base(type),
                LValue(lvalue),
                Flags(flags) {

    }

    virtual bool IsConstexpr() const { return false; }  // for optimizing branching

    virtual TYPE(ValueType<i32>) TruthinessAsCType() const {
        throw std::runtime_error("Type does not have a truthiness value: " + ToString());
    }

    virtual bool GetBoolValue() const {
        throw std::runtime_error("Type cannot be converted to bool");
    }

    virtual std::string ToString() const noexcept {
        log_fatal("Unimplemented ctype");
    };

    VIRTUAL_BINOP(Add)
    VIRTUAL_BINOP(Sub)

    VIRTUAL_BINOP(Mul)
    VIRTUAL_BINOP(Div)
    VIRTUAL_BINOP(Mod)

    VIRTUAL_BINOP(Xor)
    VIRTUAL_BINOP(BinAnd)
    VIRTUAL_BINOP(BinOr)

    TYPE(ValueType<i32>) LogAnd(const CType& other) const;
    TYPE(ValueType<i32>) LogOr(const CType& other) const;
    TYPE(ValueType<i32>) LogNot() const;

    VIRTUAL_BINOP(Lt)
    VIRTUAL_BINOP(Gt)
    VIRTUAL_BINOP(Eq)

public:
    // combinations of above functions
    CTYPE Le(const CType& other) const;
    CTYPE Ge(const CType& other) const;
    CTYPE Neq(const CType& other) const;

    VIRTUAL_BINOP(LShift)
    VIRTUAL_BINOP(RShift)

    VIRTUAL_UNOP(Deref)
    virtual UNOP_HANDLER(Ref);  // we want a different default here
    VIRTUAL_UNOP(Neg)
    VIRTUAL_UNOP(Pos)  // really just sets LValueNess to None
    VIRTUAL_UNOP(BinNot)

public:
    virtual bool CastableType(const CType& other) const {
        return false;
    }

    // for checking actual equality:
    virtual bool EqualType(const CType& other) const {
        return false;
    }

    bool IsAssignable() const { return LValue == LValueNess::Assignable; }  // if condition is used to assign to
    virtual bool HasTruthiness() const { return false; }  // if expression is used as condition

    virtual u64 ConstIntVal(bool _signed) const {
        // for array sizes
        throw std::runtime_error("Type is not an integer value: " + ToString());
    }

    virtual CTYPE Clone() const = 0;

    const BaseType Base;
    unsigned Flags = 0;

protected:
    LValueNess LValue;

    void SetNotLValue() { LValue = LValueNess::None; }         // for example the result of expressions
    void SetLValue() { LValue = LValueNess::LValue; }          // for example for const function args / struct fields
    void SetAssignable() { LValue = LValueNess::Assignable; }  // for example for non-const function args / struct fields

    virtual bool IsComplete() const {
        return false;
    }

    virtual void ForgetConstInfo() {
        // forget info on contained value (for example, adding an int to a pointer)
    }

    static TYPE(ValueType<i32>) MakeBool(bool value);
    static TYPE(ValueType<i32>) MakeBool();

    CTYPE_SIGNATURES(VIRTUAL_CASTABLE)  // compatible types (can cast)
    CTYPE_SIGNATURES(VIRTUAL_EQ)        // equal types

private:
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
};


#define OVERRIDE_BINOP_HANDLER(handler, type) BINOP_HANDLER(handler, type) override
#define OVERRIDE_BINOP_HANDLER_NOIMPL(handler, type) OVERRIDE_BINOP_HANDLER(handler, type);
#define OVERRIDE_BINOP(handler) BINOP_HANDLER(handler, CType) override { return other.R ## handler(*this); }
#define OVERRIDE_UNOP(_operator) CTYPE _operator() const override;
#define OVERRIDE_BASE_CASTABLE bool CastableType(const CType& other) const override { return other.CastableType(*this); }
#define OVERRIDE_BASE_EQ bool EqualType(const CType& other) const override { return other.EqualTypeImpl(*this); }

struct VoidType : public CType {
    explicit VoidType(unsigned flags) :
        CType(BaseType::Void, LValueNess::None, flags) {
        
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

    CTYPE Clone() const override {
        return MAKE_TYPE(VoidType)(Flags);
    }
};

template<typename T>
struct ValueType : public CType {
    explicit ValueType(LValueNess lvalue, unsigned flags) :
            CType(BaseType::Value, lvalue, flags),
            Value() {

    }

    explicit ValueType(T value, LValueNess lvalue, unsigned flags) :
            CType(BaseType::Value, lvalue, flags),
            Value(value) {

    }

    bool GetBoolValue() const override {
        if (Value.has_value()) {
            return Value.value() != 0;
        }
        throw std::runtime_error("Bool value requested from non-constant Get");
    }

    constexpr T Get() const {
        return Value.value();
    }

    constexpr bool HasValue() const {
        return Value.has_value();
    }

    bool IsConstexpr() const override { return HasValue(); }
    bool HasTruthiness() const override { return true; }

    std::string ToString() const noexcept override {
        if (!Value.has_value()) {
            return type_string_v<T>;
        }
        return type_string_v<T> + ": " + std::to_string(Value.value());
    }

    CTYPE Clone() const override {
        if (HasValue()) {
            return MAKE_TYPE(ValueType<T>)(Get(), LValue, Flags);
        }
        return MAKE_TYPE(ValueType<T>)(LValue, Flags);
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

    OVERRIDE_UNOP(Pos);
    OVERRIDE_UNOP(Neg);
    OVERRIDE_UNOP(BinNot);

    std::optional<T> Value;

protected:
    TYPE(ValueType<i32>) TruthinessAsCType() const override {
        if (HasValue()) {
            return MakeBool(Value.value() != 0);
        }
        return MakeBool();
    }

    u64 ConstIntVal(bool _signed) const override {
        if constexpr(!std::is_integral_v<T>) {
            throw std::runtime_error("Floating point type is not an integral value");
        }
        if (_signed != std::is_unsigned_v<T>) {
            throw std::runtime_error("Expected " + (_signed ? std::string() : "un") + "signed value");
        }
        return Get();
    }

    void ForgetConstInfo() override {
        Value = {};
    }

private:
    // perform BinOp on other in reverse: so this.ValueTypeRBinOp<std::plus> <==> other + this
    template<typename L, typename common_t = std::common_type_t<L, T>>
    CTYPE ValueTypeRBinOp(
            const ValueType<L>& other,
            std::function<std::common_type_t<L, T>(std::common_type_t<L, T>, std::common_type_t<L, T>)> handler
    ) const {
        // results of binary expressions are never an lvalue
        if (HasValue() && other.HasValue()) {
            return MAKE_TYPE(ValueType<common_t>)(handler(other.Get(), Get()), LValueNess::None, 0);
        }
        return MAKE_TYPE(ValueType<common_t>)(LValueNess::None, 0);
    }

    template<typename L>
    CTYPE ValueTypeRBoolBinOp(
            const ValueType<L>& other,
            std::function<i32(std::common_type_t<L, T>, std::common_type_t<L, T>)> handler
    ) const {
        if (HasValue() && other.HasValue()) {
            return MakeBool(handler(other.Get(), Get()));
        }
        return MakeBool();
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

private:
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

    bool IsComplete() const override {
        return true;
    }
};

struct PointerType : public CType {
    explicit PointerType(const CTYPE& contained, LValueNess lvalue, unsigned flags = 0) :
            CType(BaseType::Pointer, lvalue, flags),
            Contained(contained->Clone()) {

    }

    const CTYPE Contained;

    std::string ToString() const noexcept override {
        return "(" + Contained->ToString() + ") *";
    }

    bool HasTruthiness() const override { return true; }

    OVERRIDE_BASE_CASTABLE
    OVERRIDE_BASE_EQ

    OVERRIDE_BINOP(Add)
    OVERRIDE_BINOP(Sub)

    OVERRIDE_UNOP(Deref)

    CTYPE Clone() const override {
        return MAKE_TYPE(PointerType)(Contained->Clone(), LValue, Flags);
    }
protected:
    PointerType(const CTYPE& contained, BaseType base_type, LValueNess lvalue, unsigned flags = 0) :
            CType(base_type, lvalue, flags),
            Contained(contained->Clone()) {

    }

private:
    bool IsComplete() const override {
        return true;
    }

    TYPE(ValueType<i32>) TruthinessAsCType() const override {
        return MakeBool();
    }

    void ForgetConstInfo() override {
        Contained->ForgetConstInfo();
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
    explicit ArrayType(const CTYPE& contained, const CTYPE& size, unsigned flags = 0) :
            PointerType(contained, BaseType::Array, LValueNess::LValue, flags) {
        // arrays are lvalues, but not assignable (besides the initializer)
        Size = size->ConstIntVal(false);
    }

    explicit ArrayType(const CTYPE& contained, size_t size, unsigned flags = 0) :
            PointerType(contained, BaseType::Array, LValueNess::LValue, flags),
            Size(size) {

    }

    size_t Size;

    std::string ToString() const noexcept override {
        return "(" + Contained->ToString() + ")[" + std::to_string(Size) + "]";
    }

    CTYPE Clone() const override {
        return MAKE_TYPE(ArrayType)(Contained, Size, Flags);
    }

private:
    bool IsComplete() const override {
        return true;
    }
};

struct FunctionType : public PointerType {
    explicit FunctionType(const CTYPE& return_type, std::string symbol = "", unsigned flags = 0) :
            PointerType(
                    return_type,
                    BaseType::Function,
                    symbol.empty() ? LValueNess::Assignable : LValueNess::LValue,
                    flags
                    ),
            Symbol(std::move(symbol)) {
        Contained->ForgetConstInfo();
        // functions are assignable if they are variables, but not if they are global symbols
    }

    const std::string Symbol;  // if there is a symbol here, the function is a global definition
    std::vector<CTYPE> ArgTypes;

    void AddArg(const CTYPE& arg) {
        ArgTypes.push_back(arg->Clone());
        ArgTypes.back()->ForgetConstInfo();  // constant info is nonsense for arguments
    }

    TYPE(ValueType<i32>) TruthinessAsCType() const override {
        if (!Symbol.empty()) {
            return MakeBool(true);  // always true
        }
        return MakeBool();  // unknown, might be function pointer variable
    }

    std::string ToString() const noexcept override {
        std::string repr = Contained->ToString() + " " + Symbol + "(";
        for (auto& a : ArgTypes) {
            repr += a->ToString() + ",";
        }
        return repr + ")";
    }

    CTYPE Clone() const override {
        auto clone = MAKE_TYPE(FunctionType)(Contained, Symbol, Flags);
        for (const auto& arg : ArgTypes) {
            clone->AddArg(arg);
        }
        return clone;
    }

    OVERRIDE_BASE_CASTABLE
    OVERRIDE_BASE_EQ

    // what was I doing here?:
//    OVERRIDE_BINOP(Lt);
//    OVERRIDE_BINOP(Gt);
//    OVERRIDE_BINOP(Eq);

    OVERRIDE_UNOP(Deref)

private:
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

    bool EqualTypeImpl(const PointerType& other) const override { return false; }
    bool EqualTypeImpl(const FunctionType& other) const override {
        if (!(*Contained).EqualType(*other.Contained)) {
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

    bool IsComplete() const override {
        return true;
    }
};

template<CType::BaseType type>
struct StructUnionType : public CType {
    explicit StructUnionType(LValueNess lvalue, unsigned flags = 0) : CType(type, lvalue, flags) {

    }

    struct Field {
        Field(std::string name, size_t size, const CTYPE& contained) :
            Name(std::move(name)),
            Size(size),
            Type(contained->Clone()) {

        }

        Field(std::string name, const CTYPE& contained) :
            Name(std::move(name)),
            Size(0),
            Type(contained->Clone()) {

        }

        const std::string Name;
        const size_t Size = 0;  // 0 means default size
        std::shared_ptr<CType> Type;
    };

    void AddField(const std::string& name, size_t size, const CTYPE& contained) {
        Fields.push_back(Field(name, size, contained));
    }

    void AddField(const std::string& name, const CTYPE& contained) {
        Fields.push_back(Field(name, contained));
    }

    CTYPE Clone() const override {
        auto clone = MAKE_TYPE(StructUnionType<type>)(LValue, Flags);
        for (const auto& arg : Fields) {
            clone->AddField(arg.Name, arg.Size, arg.Type);
        }
        return clone;
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

    bool IsComplete() const override {
        return !Fields.empty();
    }

    void ForgetConstInfo() override {
        for (auto& field : Fields) {
            field.Type->ForgetConstInfo();
        }
    }
};

struct StructType : public StructUnionType<CType::BaseType::Struct> {
    explicit StructType(LValueNess lvalue, unsigned flags = 0) : StructUnionType(lvalue, flags) {

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
    explicit UnionType(LValueNess lvalue, unsigned flags = 0) : StructUnionType(lvalue, flags) {

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
