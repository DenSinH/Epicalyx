#ifndef EPICALYX_TOKEN_TYPES_H
#define EPICALYX_TOKEN_TYPES_H

enum class TokenClass {
    Identifier,
    Keyword,
    Punctuator,
    StringConstant,
    NumericalConstant,
};


enum PunctuatorFlags : unsigned {
    None         = 0x0,
    UnOp         = 0x1,
    BinOp        = 0x2,
    PostFix      = 0x4,
    MemberAccess = 0x8,
    Relational   = 0x10,
    Assignment   = 0x20,
};

enum class TokenType {
    Identifier,

    // constants
    ConstFloat,
    ConstDouble,
    ConstInt,
    ConstUnsignedInt,
    ConstLong,
    ConstUnsignedLong,
    ConstLongLong,
    ConstUnsignedLongLong,
    ConstString,

    // Keywords
    Auto,
    Break,
    Case,
    Char,
    Const,
    Continue,
    Default,
    Do,
    Double,
    Else,
    Enum,
    Extern,
    Float,
    For,
    Goto,
    If,
    Inline,
    Int,
    Long,
    Register,
    Restrict,
    Return,
    Short,
    Signed,
    Sizeof,
    Static,
    Struct,
    Switch,
    Typedef,
    Union,
    Unsigned,
    Void,
    Volatile,
    While,
    Alignas,
    Alignof,
    Atomic,
    Bool,
    Complex,
    Generic,
    Imaginary,
    Noreturn,
    StaticAssert,
    ThreadLocal,

    // Punctuators
    LBracket,       // [
    RBracket,       // ]
    LParen,         // (
    RParen,         // )
    LBrace,         // {
    RBrace,         // }
    Dot,            // .
    Arrow,          // ->
    Incr,           // ++
    Decr,           // --
    Ampersand,      // &
    Asterisk,       // *
    Plus,           // +
    Minus,          // -
    Tilde,          // ~
    Exclamation,    // !
    Div,            // /
    Mod,            // %
    LShift,         // <<
    RShift,         // >>
    Less,           // <
    Greater,        // >
    LessEqual,      // <=
    GreaterEqual,   // >=
    Equal,          // ==
    NotEqual,       // !=
    BinXor,         // ^
    BinOr,          // |
    LogicalAnd,     // &&
    LogicalOr,      // ||
    Question,       // ?
    Colon,          // :
    SemiColon,      // ;
    Ellipsis,       // ...
    Assign,         // =
    IMul,           // *=
    IDiv,           // /=
    IMod,           // %=
    IPlus,          // +=
    IMinus,         // -=
    ILShift,        // <<=
    IRShift,        // >>=
    IAnd,           // &=
    IXor,           // ^=
    IOr,            // |=
    Comma,          // ,
    Hashtag,        // #
    HHashtag,       // ##
    // <: :> <% %> %: %:%: I don't know what these are
};

#endif //EPICALYX_TOKEN_TYPES_H