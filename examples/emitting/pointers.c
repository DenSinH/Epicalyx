int main() {
  int* test = 0;
  int* test2 = 256;
  int a = 12;
  test++;
  test += 12;
  if (test > 1) {
    return (test + 1) > test2;
  }
  else {
    return test - 1;
  }
}