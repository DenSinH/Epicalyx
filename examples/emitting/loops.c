
int main() {
  int x = 0;
  int y = 1;
  while (++x < 4) {
    y *= y + 1;
  }

  do {
    y--;
  } while (x++ < 6);

  return y;
}