/*
PATTERN:
0025-bad-init.c:7: warning: initializer-string for array of chars is too long
.
*/

char s2[2] = "foo";

int
main()
{
	return 0;
}
