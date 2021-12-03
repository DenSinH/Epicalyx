
int main() {
  int x = 0;
  int y = 1;
  if (x++) {
    return 1;
  }
  else if (y - 1 > x) {
    return 2;
  }
  else if (!--y) {
    return 12;
  }
  else {
    return 3;
  }
  return 0;
}