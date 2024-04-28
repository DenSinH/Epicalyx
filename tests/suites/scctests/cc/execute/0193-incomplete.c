struct dummy;

void *
fun(struct dummy p[])
{
	return p;
}

int
main()
{
	return 0 != fun(0);
}
