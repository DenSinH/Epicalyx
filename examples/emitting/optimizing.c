int main() {
  int i = (float)0;

  int j = -(-i);
  j += -(-(-i));

  if (i == 0) {
    return (i + 1) << 1;
  }
  return i == 1;
}