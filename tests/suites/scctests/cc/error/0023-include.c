/*
 * Test a comment that goes beyond of the end of an
 * included file
PATTERN:
0023-include.h:8: error: unterminated comment
0023-include.c:10: error: #endif expected
.
*/

#include "0023-include.h"
