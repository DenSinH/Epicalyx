/*
PATTERN:
0030-krtypes.c:15: error: too many arguments in function call
.
*/

static void foo()
{
}

void bar();

int main()
{
	foo(0);
	bar(0);
	return 0;
}
