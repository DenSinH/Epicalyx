typedef struct s_struct{
    int a, b : 12;
    void (*c)(int);
} s_struct;

_Static_assert(sizeof(s_struct) == 16.0, "this is a test");

s_struct my_function() {
    return (s_struct){
        .a = 1,
        .b = 2,
        .c = (void (*)(int))0
    };
}

