int main() {
  int x = 0, y = {1};
  char z;
  y = x = 2;
  z = y;
  y = z;
  return 0;
}