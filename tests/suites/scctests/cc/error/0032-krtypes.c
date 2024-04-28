/*
PATTERN:
0032-krtypes.c:21: error: too many arguments in function call
.
*/

int f();

int bar()
{
    return f(0);
}

int f()
{
    return 0;
}

int z()
{
    return f(0);
}
