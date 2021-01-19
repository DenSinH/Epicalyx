int test() {
    char normal = 'b';
    int multiple = 'abc';
    int multiple_escape = 'ab\n';
    multiple_escape = '\'\"\?\\';
    multiple_escape = '\a\b\f\n';
    multiple_escape = '\r\t\v';
    int char_terminator_escape = '\'';
    char_terminator_escape = '\'\'\'';

    int multiple_hex_escape = '\x11\xab\xAB';
    int multiple_oct_escape = '\1\12\123';
    multiple_oct_escape = '\6\34\153';
    multiple_oct_escape = '\7\38\177';

    const char* string = "hello world!";
    const char* escape_string = "hello\nworld!\t\a\b\?\\\test\"\"\'";
    const char* hex_escape_string = "hello\x12\x31\xAB\xbcworld!";

    const int a = 123;
    switch(multiple_escape) {
        case a == 123 ? 1 : multiple_escape:
            return 0;
    }
}