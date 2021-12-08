int main() {
  int array[12];
  *array = 1;
  if (array[1]) {
    array[2] = 3;
  }
  else {
    array[2] = 4;
  }

  array[array[2]] = 2;
  array[0]++;
  0[array]++;
  int* test = &array[0];
  test += 2;
  return 0[array];
}