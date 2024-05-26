## TODO:
 - some pre-processor directives
 - struct/union sized fields
 - optimize `tests/suites/scctests/cc/execute/0033-ptrindec.c` example
 - ExpressionParser cast -> unary in `0160-cpp_if.c`
 - optimize pointer adds / loc addrs in `0139-ptr_ary.c`
 - Readonly globals + string to global ID mapping for preventing duplicates (for example `0200-cpp.c`)
 - Short CString hash (directly loading `direct` field as `size_t` for the hash, useful for Preprocessor.Stddef standard macros, or mapping c stl functions).
 - Iterator for "current" object, calling next "current" object, and allowing you to find a struct field in a struct definition. This solves the "nested declarator" problem, and allows to easily define Struct / Union initializer lists / nested initializer lists. The end of a (nested) initializer list just skips to the next "parent" field.
 - Generalize AddGlobal stuff in ASTWalker (as of now, about 3 things need to happen, which is excessive, since it is re-used in 2 places).
   
## Unsupported
 - `_Generic` related statements
 - Empty argument names for external function definitions
```C
void func(int) {
  return;
}
```
 - Array size `static` and `type-specifier` declarations (parsed, but not dealt with)
```C
int test[static 12];
```
 - Identifier lists in K&R style functions
```C
void func(identifier-list) declaration-list {
  // body
}
```
e.g:
```C
void func(x) int x;
```
 - Anonymous struct definitions (parsed, but seen as incomplete)
```C
typedef struct {} s;

int main() {
  // should return 0  
  return sizeof(s);
}
```