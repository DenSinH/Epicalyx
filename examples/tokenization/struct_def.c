int test(int a, float b) {
    struct {
        int a, b;
        float c;
    } abc;
    a++;
    a--;
    (&a)[12];
    return (a * b) || abc.c;
}