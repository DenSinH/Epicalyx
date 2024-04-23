int func() {
  return 1;
}

int main() {
  int test = func();
  test += 1 + 2 + 3;
  test += 1;
  if (test == 2) {
    return test == 2;
  }
  return test;
}