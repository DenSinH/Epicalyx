#undef macro
# define macro2
#
#define macro3 12 + 1
// comment
/*
 * multiline comment
 * */

#define functional_macro(arg) (arg + 1)

int global = 0;

/* comment */void test() {
#if (macro3) == 13
  global = 1;
#elif 0 // this is a comment\
that goes up to here
#elifdef macro2
#ifdef macro3
  global = 0;
#else
#define define_in_disabled_group
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
#ifdef define_in_disabled_group
  return -123
#endif
  test();
  double one_added = add_half(add_half(global));
  return functional_macro(add_half(add_half(one_added)));
}