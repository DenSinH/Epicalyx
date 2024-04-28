#include <limits.h>

struct stk {
	int f1;
	int f2;
};

int
main()
{
	static struct stk s = {LLONG_MAX+1};

	return s.f2;
}
