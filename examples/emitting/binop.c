int main() {
  int x = 0, y = 1;
  long long result;
  result = x;
  result = 5.0 * (0 + x - y);
  result = -result;
  result = ~result;
  result = result % (13 + x);
  return result;
}