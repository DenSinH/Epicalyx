/*
PATTERN:
0026-numargs.c:18: error: incompatible type for argument 1 in function call
0026-numargs.c:19: error: incompatible type for argument 2 in function call
0026-numargs.c:20: error: too many arguments in function call
.
*/

int
fn(int a, int b)
{
	return a+b;
}

int
main(void)
{
	fn("a", 1);
	fn(1, "a");
	fn(1, 2, 3);

	return 0;
}
