
int main() {
  int x = 0;
  int y = 1;
  while (++x < 4) {
    y *= y + 1;
  }

  do {
    y--;
  } while (x++ < 6);

  for (int i = 0; i < 10; i++) {
    y += i;
    if (i > 5) {
      goto done;
    }
  }

done:
  return y;
}