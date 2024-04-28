/*
PATTERN:
0031-krtypes.c:32: error: incompatible types when assigning
0031-krtypes.c:33: error: incompatible types when assigning
0031-krtypes.c:34: error: incompatible types when assigning
.
*/

int
f1(char c)
{
	return c;
}

int
f2(float c)
{
	return c;
}

int
f3(int a, ...)
{
	return a;
}

int
main()
{
	int (*fp)();

	fp = f1;
	fp = f2;
	fp = f3;

	return 0;
}
