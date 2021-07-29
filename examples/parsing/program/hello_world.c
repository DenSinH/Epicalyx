extern int printf(const char*, ...);

int main();
extern int test;

typedef struct s_test_struct {
  int x, y, z;
  char* const p;
} s_test_struct;

int assign(int a, int* b) {
  *b = a;
}

int main() {
  int* a = {0, 0};

  s_test_struct x;
  x.x = (int){0 + 1 + 1.2 * 0.5};
  x.p[0] = 'h';

  if (0) printf("Sup\n");
  printf("hello world!");
  return 1;
}