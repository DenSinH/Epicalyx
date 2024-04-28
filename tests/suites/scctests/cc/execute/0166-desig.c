struct S {
	int a, b, c;
	char d[3];
	int e;
} s = {
	.a = 1,
	.b = 2,
	.d = {[0] = 3, [2] = 5},
	.d = {[0] = 4, [1] = 6}
};

int
main(void)
{
	return 0 == s.d[2];
}
