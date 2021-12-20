int global = 0;

void test() {
  global = 1;
}

float add_half(float a) {
  return a + 0.5;
}

int main() {
  test();
  double one_added = add_half(add_half(global));
  return add_half(add_half(one_added));
}