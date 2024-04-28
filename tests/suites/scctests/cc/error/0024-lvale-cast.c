/*
PATTERN:
0024-lvale-cast.c:12: error: lvalue required in operation
.
*/

int
main()
{
	int a;

	(char) a = 1025;

	return (a == 1) ? 0 : 1;
}
