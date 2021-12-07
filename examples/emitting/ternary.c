int main() {
  int x;
  int c = 1;
  if (c ? 1 : 0) {
    x = c ? 2 : 0.5;
  }
  else {
    x = c ? 3 : 1.0;
  }
  return x;
}