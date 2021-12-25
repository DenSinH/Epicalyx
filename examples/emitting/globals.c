int x;
int y;
int z = 1;
int* ptr = &z + 1;

int test() {
  return 0;
}

int main() {
  x = 0;
  y = 2;
  x += y;
  int** temp = &ptr;
  int x = *temp;
  return *(*temp - 1);
}