#undef macro
# define macro2 /*
*/#if true 
#define macro3 12 + \
  \
  1
// comment
/*
 * multiline comment
 * */

#define no_arg_macro() 0
#define va_arg_marcro(...) __VA_ARGS__
#define functional_macro(arg) (arg + no_arg_macro())

int global = 0;
 // this is a comment\
that goes up to here
/* comment */void test() {
#if (macro3) == 12
  global = 1;
#elif 0
#elifdef macro2
#ifndef macro3
  global = 0;
#else
#define define_in_nested_group
  global = 42;
#endif
#else
  global = -1;
#endif
}

float add_half(float a) {
  return a + 0.5;
}

int main() {
  test();
  double one_added = add_half(add_half(global));
  return functional_macro(add_half(add_half(one_added)))
#ifdef define_in_nested_group
  + va_arg_marcro(__LINE__)
#endif
  ;
}