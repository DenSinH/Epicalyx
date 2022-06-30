#undef macro
# define macro2
#
#define macro3
// comment
/*
 * multiline comment
 * */

int global = 0;

/* comment */void test() {
#ifdef macro
  global = 1;
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
  return add_half(add_half(one_added));
}