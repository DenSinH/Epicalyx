/*
PATTERN:
0033-character.c:23: error: empty character constant
0033-character.c:24: warning: multi-character character constant
0033-character.c:25: error: empty character constant
0033-character.c:26: warning: multi-character character constant
0033-character.c:27: warning: multi-character character constant
0033-character.c:28: error: invalid multibyte sequence
0033-character.c:28: warning: multi-character character constant
0033-character.c:29: error: invalid multibyte sequence
0033-character.c:29: error: invalid multibyte sequence
0033-character.c:29: warning: multi-character character constant
.
*/
#include <wchar.h>

int
main()
{
	int i;
	wchar_t w;

	i = '';
	i = 'ab';
	w = L'';
	w = L'ab';
	w = L'รกรก';
	w = L'€ ';
	w = L'€ภ';

	return 0;
}
