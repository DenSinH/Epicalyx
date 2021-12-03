int main() {
  int x = 0;
  int y = 0;
  switch (x) {
    case 0:
      y = 1;
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