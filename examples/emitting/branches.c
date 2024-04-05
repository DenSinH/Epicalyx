
int test(int x, int y) {
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

int main() {
  return test(0, 1);
}