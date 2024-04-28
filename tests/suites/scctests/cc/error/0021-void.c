/*
PATTERN:
0021-void.c:12: error: bad type conversion requested
.
*/

int
main(void)
{
	void f(void);

	return (int) f();
}
