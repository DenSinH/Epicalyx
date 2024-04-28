/*
PATTERN:
0036-func.c:8: error: __func__ is a reserved variable name
0036-func.c:10: error: __func__ is a reserved variable name
0036-func.c:13: warning: '__func__' defined but not used
.
*/
int __func__;

int foo(int __func__)
{
	return 0;
}
