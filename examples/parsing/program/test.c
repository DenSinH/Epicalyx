
static char is_prime(const int n) {
    for (int i = 0; i * i < n; i++) {
        if ((n % i) == 0) {
            return 0;
        }
    }
    return 1;
}

extern void bruh(), yo(), more_functions(int lol);

int test(int a) {
    switch(a) {
        case 0:
            return 12;
        case 12:
            return 12123;
        case 1 + 3 + 4:
            return 1;
        case 1 + 2 + 3:
            return 123123;
        default:
            return a - 1;
    }
}

int main() {
    return is_prime(17);
}