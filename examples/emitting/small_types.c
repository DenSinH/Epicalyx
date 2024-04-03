
int test() {
  char x = -1;
  x >>= 8;
  return x;
}

int test2() {
  char x = 1;
  return x << 8;
}

int main() {
  char x = 1;
  x <<= 8;
  return x;
}