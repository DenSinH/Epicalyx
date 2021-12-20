int test() {
  return 1;
}

float add_half(float a) {
  return a + 0.5;
}

int main() {
  double one_added = add_half(add_half(test()));
  return add_half(add_half(one_added));
}