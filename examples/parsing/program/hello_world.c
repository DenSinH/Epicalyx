extern int printf(const char*, ...);

int main();
extern int test;

typedef struct s_test_struct {
  int x, y, z;
  char* const p;
} s_test_struct;


int main() {
  int* a = {0, 0};

  s_test_struct x;
  x.x = 0;
  x.p[0] = 'h';

  if (1) printf("Sup\n");
  printf("hello world!");
  return 1;
}