/*
PATTERN:
0020-storage.c:36: warning: 'par' defined but not used
0020-storage.c:42: error: incorrect storage class for file-scope declaration
0020-storage.c:45: error: bad storage class in function parameter
0020-storage.c:46: error: invalid storage class for function 'func4'
0020-storage.c:47: error: invalid type specification
0020-storage.c:53: error: conflicting types for 'd'
.
*/

int a;
static char b;
extern int c;
typedef unsigned e;

int
func1(void)
{
        auto h;
        static char i;
        register long j;
        extern int k;
        static unsigned long a;
        return h+i+j+k+a;
}

int
func2(register int par)
{
        int par;

	return par;
}

static int
func3(register int par)
{
	return par;
}

register short d;

register int
func4(static int par)
{
        static register f;

	return f+par;
}

short d;
char d;
