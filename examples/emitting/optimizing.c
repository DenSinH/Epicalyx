int func() {
  return 1;
}

int main() {
  int test = func();
  test += 1;
  test += 1;
  return test;
}