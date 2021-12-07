
int main() {
  int x = 0;
  int y = 1;
  while (++x < 4) {
    y *= y + 1;
    y <<= 1;
    break;
  }

  do {
    y--;
  } while (x++ < 6);

  for (int i = 0; i < 10; i++) {
    y += i;
    continue;
    if (i > 5) {
      goto done;
    }
  }

  if (!y ? 0 : 1) {
    y += 1;
  }

done:
  return y;
}