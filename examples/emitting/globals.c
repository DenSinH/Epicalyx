int x;
int y;
int z = 1;

int test() {
  return 0;
}

int main() {
  x = 0;
  y = 2;
  x += y;
  int* temp = &z;
  int x = *temp;
  return x;
}