int test(int x) {
  int y = 0;
  switch (x) {
    case 0:
      y = 1;
    case 1:
      y = 2;
    case 2:
      y += 1;
      break;
    case 4:
      return 123;
    default:
      y = 2;
      break;
  }
  return y;
}

int main() {
  return test(12);
}