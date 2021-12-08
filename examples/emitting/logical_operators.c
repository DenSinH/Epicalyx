int main() {
  int x = 0;
  int y = 1;
  if (x == 0 && y == 0) {
    return 0;
  }
  else if (x == 0 || y == 0) {
    return (x == 0 || y == 0) + (x == 0 && (x + 1) == 1);
  }
  else {
    return 3;
  }
}