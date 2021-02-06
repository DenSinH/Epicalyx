#include "types.h"

BINOP_HANDLER(PointerType::RSub, PointerType) {
    if ((*this).EqualType(other)) {
        return MAKE_TYPE(ValueType<u64>)(0);
    }
    throw std::runtime_error("Cannot subtract pointers of non-compatible types");
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i8>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u8>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u16>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<i16>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u32>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<i32>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u64>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<i64>) { return MAKE_TYPE(PointerType)(Contained, Flags); }

BINOP_HANDLER(PointerType::RSub, ValueType<i8>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RSub, ValueType<u8>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RSub, ValueType<u16>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RSub, ValueType<i16>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RSub, ValueType<u32>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RSub, ValueType<i32>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RSub, ValueType<u64>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RSub, ValueType<i64>) { return MAKE_TYPE(PointerType)(Contained, Flags); }