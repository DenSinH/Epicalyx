/*
PATTERN:
0035-cpp.c:8: error: '##' cannot appear at either ends of a macro expansion
0035-cpp.c:9: error: '##' cannot appear at either ends of a macro expansion
.
*/

#define M(x) x##
#define N(x) ##x
