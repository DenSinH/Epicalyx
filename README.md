## TODO:
 - some pre-processor directives
 - struct/union sized fields
 - optimize `tests/suites/scctests/cc/execute/0033-ptrindec.c` example
 - ExpressionParser cast -> unary in `0160-cpp_if.c`
 - optimize pointer adds / loc addrs in `0139-ptr_ary.c`
   
## Unsupported
 - `_Generic` related statements
 - Empty argument names for external function definitions
```C
void func(int) {
  return;
}
```
 - Array size `static` and `type-specifier` declarations
```C
int test[static 12];
```
 - Identifier lists in K&R style functions
```C
void func(identifier-list)
declaration-list
{
    body
}
```
e.g:
```C
void func(x) int x;
```