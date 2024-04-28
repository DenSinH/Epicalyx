/*
PATTERN:
0037-pointer.c:11: error: incorrect type in return
.
*/

char *
f()
{
	return 1, 0;
}
