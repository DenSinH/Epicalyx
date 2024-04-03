
int test(int a, float b) {
  float c = a + b;
  return c - a;
}

int main() {
  return test(1, 2.0);
}